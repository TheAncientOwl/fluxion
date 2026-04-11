/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FileDialog.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see Graphite/Common/UI/FileDialog.hpp
///

#include "Graphite/Common/UI/FileDialog.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include "IconsCodicons.h"
#include "imgui.h"

namespace Graphite::Common::UI {

FileDialog::FileDialog() = default;

void FileDialog::Open(
    const std::string& title,
    EFileDialogMode mode,
    const std::filesystem::path& initial_path,
    const std::vector<FileFilter>& filters)
{
    m_state.title = title;
    m_state.mode = mode;
    m_state.initial_path = initial_path;
    m_state.current_path = initial_path;
    m_state.filters = filters;
    m_state.selected_paths.clear();
    m_state.is_open = true;
    m_state.show_dialog = true;
}

bool FileDialog::Render()
{
    if (!m_state.is_open)
        return false;

    bool is_still_open{true};

    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(m_state.title.c_str(), &is_still_open, ImGuiWindowFlags_NoCollapse))
    {
        if (ImGui::BeginChild("FileDialogChild", ImVec2(0, -50), ImGuiChildFlags_Borders))
        {
            RenderPathBar();
            ImGui::Separator();

            if (ImGui::BeginChild("FileList", ImVec2(0, -40), ImGuiChildFlags_Borders))
            {
                RenderFileList();
                ImGui::EndChild();
            }

            RenderFilterSelector();
        }
        ImGui::EndChild();

        ImGui::Spacing();
        RenderActionButtons();
    }
    ImGui::End();

    if (!is_still_open)
    {
        m_state.is_open = false;
        m_state.selected_paths.clear();
    }

    return m_state.is_open;
}

bool FileDialog::IsOpen() const
{
    return m_state.is_open;
}

const std::filesystem::path& FileDialog::GetSelectedPath() const
{
    static const std::filesystem::path empty;
    return m_state.selected_paths.empty() ? empty : m_state.selected_paths.front();
}

const std::vector<std::filesystem::path>& FileDialog::GetSelectedPaths() const
{
    return m_state.selected_paths;
}

void FileDialog::SetSelectionCallback(std::function<void(const FileDialogResult&)> callback)
{
    m_selection_callback = std::move(callback);
}

void FileDialog::Close()
{
    m_state.is_open = false;
    m_state.selected_paths.clear();
}

void FileDialog::RenderPathBar()
{
    ImGui::Text("Location: ");
    ImGui::SameLine();

    char path_buffer[512];
    std::strncpy(path_buffer, m_state.current_path.string().c_str(), sizeof(path_buffer) - 1);
    path_buffer[sizeof(path_buffer) - 1] = '\0';

    ImGui::SetNextItemWidth(-40.0f);
    if (ImGui::InputText(
            "##current_path", path_buffer, sizeof(path_buffer), ImGuiInputTextFlags_ReadOnly))
    {
        // Path is read-only, just for display
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_CI_HOME))
    {
        m_state.current_path = std::filesystem::path(std::getenv("HOME"));
    }
}

void FileDialog::RenderFileList()
{
    try
    {
        if (!std::filesystem::exists(m_state.current_path))
        {
            ImGui::TextDisabled("Directory not found");
            return;
        }

        // Display parent directory
        if (m_state.current_path != m_state.current_path.root_path())
        {
            if (ImGui::Selectable(
                    ICON_CI_ARROW_UP " ..",
                    false,
                    ImGuiSelectableFlags_None,
                    ImVec2(ImGui::GetContentRegionAvail().x, 0)))
            {
                m_state.current_path = m_state.current_path.parent_path();
                m_state.selected_paths.clear();
            }
        }

        // Display all entries
        std::vector<std::filesystem::directory_entry> entries;
        for (const auto& entry : std::filesystem::directory_iterator(m_state.current_path))
        {
            entries.push_back(entry);
        }

        // Sort entries: directories first, then alphabetically
        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
            if (a.is_directory() != b.is_directory())
                return a.is_directory() > b.is_directory();
            return a.path().filename().string() < b.path().filename().string();
        });

        for (const auto& entry : entries)
        {
            const auto& path = entry.path();
            std::string display_name = path.filename().string();

            if (entry.is_directory())
            {
                display_name = std::string(ICON_CI_FOLDER) + " " + display_name;
            }
            else
            {
                display_name = std::string(ICON_CI_FILE) + " " + display_name;
            }

            bool should_show = true;
            if (entry.is_regular_file() && !m_state.filters.empty())
            {
                should_show = PathMatchesFilter(path);
            }

            if (should_show)
            {
                // Check if this entry is currently selected
                bool is_selected =
                    !m_state.selected_paths.empty() && m_state.selected_paths.front() == path;

                if (ImGui::Selectable(
                        display_name.c_str(),
                        is_selected,
                        ImGuiSelectableFlags_None,
                        ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                {
                    if (entry.is_directory())
                    {
                        m_state.current_path = path;
                        m_state.selected_paths.clear();
                    }
                    else
                    {
                        m_state.selected_paths.clear();
                        m_state.selected_paths.push_back(path);
                    }
                }

                // Allow double-click on directories to navigate
                if (entry.is_directory() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    m_state.current_path = path;
                    m_state.selected_paths.clear();
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", e.what());
    }
}

void FileDialog::RenderFilterSelector()
{
    if (m_state.filters.empty())
        return;

    ImGui::Spacing();
    ImGui::Text("File Type: ");
    ImGui::SameLine();

    static int selected_filter = 0;
    std::vector<const char*> filter_names;
    for (const auto& filter : m_state.filters)
    {
        filter_names.push_back(filter.name.c_str());
    }

    ImGui::SetNextItemWidth(200.0f);
    ImGui::Combo(
        "##file_filters", &selected_filter, filter_names.data(), static_cast<int>(filter_names.size()));
}

void FileDialog::RenderActionButtons()
{
    float button_width = 120.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float available_width = ImGui::GetContentRegionAvail().x;

    ImGui::SetCursorPosX(available_width - (button_width * 2 + spacing));

    bool should_confirm = false;

    if (ImGui::Button("Select", ImVec2(button_width, 0)))
    {
        should_confirm = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(button_width, 0)))
    {
        m_state.is_open = false;
        m_state.selected_paths.clear();
    }

    if (should_confirm && !m_state.selected_paths.empty())
    {
        FileDialogResult result;
        result.path = m_state.selected_paths.front();
        result.was_selected = true;

        if (m_selection_callback)
        {
            m_selection_callback(result);
        }

        m_state.is_open = false;
    }
}

bool FileDialog::PathMatchesFilter(const std::filesystem::path& path) const
{
    if (m_state.filters.empty())
        return true;

    std::string extension = GetFileExtension(path);

    for (const auto& filter : m_state.filters)
    {
        for (const auto& filter_ext : filter.extensions)
        {
            if (extension == filter_ext)
                return true;
        }
    }

    return false;
}

std::string FileDialog::GetFileExtension(const std::filesystem::path& path) const
{
    std::string ext = path.extension().string();
    // Convert to lowercase for case-insensitive comparison
    std::transform(
        ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    return ext;
}

} // namespace Graphite::Common::UI
