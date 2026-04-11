/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FileDialog.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief File dialog UI component for selecting files and directories.
///

#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace Graphite::Common::UI {

/**
 * @brief File filter for file dialogs
 */
struct FileFilter
{
    std::string name; ///< Display name (e.g., "Image Files")
    std::vector<std::string> extensions; ///< File extensions (e.g., {".png", ".jpg"})
};

/**
 * @brief File dialog result
 */
struct FileDialogResult
{
    std::filesystem::path path;
    bool was_selected{false};
};

/**
 * @brief File dialog mode
 *
 */
enum class EFileDialogMode
{
    OpenFile, ///< Select a single file to open
    OpenMultiple, ///< Select multiple files
    OpenDirectory, ///< Select a directory
    SaveFile, ///< Select a file location for saving
};

/**
 * @brief File dialog component for choosing files or directories
 *
 */
class FileDialog
{
public:
    FileDialog();
    ~FileDialog() = default;

    /**
     * @brief Opens the file dialog
     *
     * @param title Dialog window title
     * @param mode Dialog mode (open, save, directory, etc.)
     * @param initial_path Starting directory path
     * @param filters File filters to apply
     */
    void Open(
        const std::string& title,
        EFileDialogMode mode = EFileDialogMode::OpenFile,
        const std::filesystem::path& initial_path = std::filesystem::current_path(),
        const std::vector<FileFilter>& filters = {});

    /**
     * @brief Renders the file dialog UI
     *
     * @return True if the dialog should remain open, false if closed
     */
    bool Render();

    /**
     * @brief Self Explanatory
     */
    bool IsOpen() const;

    /**
     * @brief Self Explanatory
     */
    const std::filesystem::path& GetSelectedPath() const;

    /**
     * @brief Gets all selected file paths (for multi-select mode)
     */
    const std::vector<std::filesystem::path>& GetSelectedPaths() const;

    /**
     * @brief Sets a callback when files are selected
     */
    void SetSelectionCallback(std::function<void(const FileDialogResult&)> callback);

    /**
     * @brief Closes the dialog without selection
     */
    void Close();

private:
    struct State
    {
        std::string title;
        EFileDialogMode mode{EFileDialogMode::OpenFile};
        std::filesystem::path current_path;
        std::filesystem::path initial_path;
        std::vector<FileFilter> filters;
        std::vector<std::filesystem::path> selected_paths;
        bool is_open{false};
        bool show_dialog{false};
    };

    State m_state;
    std::function<void(const FileDialogResult&)> m_selection_callback;

    void RenderFileList();
    void RenderPathBar();
    void RenderFilterSelector();
    void RenderActionButtons();

    bool PathMatchesFilter(const std::filesystem::path& path) const;
    std::string GetFileExtension(const std::filesystem::path& path) const;
};

} // namespace Graphite::Common::UI
