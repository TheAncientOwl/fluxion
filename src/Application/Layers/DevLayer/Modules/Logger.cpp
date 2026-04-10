/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Logger.cpp
/// @author Alexandru Delegeanu
/// @version 0.7
/// @brief Implementation of @see Logger.hpp.
///

#include <algorithm>
#include <regex>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "IconsCodicons.h"
#include "imgui.h"

#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Layers::Modules::DevLayer::Logger);
USE_LOG_SCOPE(Fluxion::Application::Layers::Modules::DevLayer::Logger);

namespace Fluxion::Application::Layers::Modules::DevLayer {

// namespace CPPRenderer {

// enum class SigTokenType
// {
//     Keyword,
//     Identifier,
//     Namespace,
//     Class,
//     Method,
//     Punctuation,
//     Whitespace,
//     Number,
//     Unknown
// };

// struct Token
// {
//     SigTokenType type;
//     std::string_view text;
// };

// constexpr size_t MaxSigTokens = 200;

// bool IsKeyword(std::string_view const word) noexcept
// {
//     static constexpr const char* keywords[] = {
//         "auto",   "bool",   "char",     "class",    "const",    "constexpr", "default",
//         "delete", "double", "enum",     "explicit", "extern",   "float",     "friend",
//         "inline", "int",    "long",     "noexcept", "operator", "short",     "static",
//         "struct", "switch", "template", "unsigned", "virtual",  "void",
//     };

//     for (auto k : keywords)
//     {
//         if (word == k)
//         {
//             return true;
//         }
//     }
//     return false;
// }

// bool IsIdentifierChar(char const c) noexcept
// {
//     return (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
// }

// size_t TokenizeCppSignature(std::string_view line, Token (&tokens)[MaxSigTokens])
// {
//     size_t token_count = 0;
//     size_t pos = 0;
//     size_t len = line.size();

//     // Temporary storage for identifiers and their positions to adjust types after processing
//     '::' struct IdentPos
//     {
//         size_t index; // index in tokens array
//     };
//     std::vector<IdentPos> identifier_positions;

//     while (pos < len && token_count < MaxSigTokens)
//     {
//         char c = line[pos];

//         // Whitespace
//         if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
//         {
//             size_t start = pos;
//             while (pos < len &&
//                    (line[pos] == ' ' || line[pos] == '\t' || line[pos] == '\n' || line[pos] == '\r'))
//                 ++pos;
//             tokens[token_count++] = Token{SigTokenType::Whitespace, line.substr(start, pos - start)};
//             continue;
//         }

//         // Identifier or keyword
//         if ((c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
//         {
//             size_t start = pos;
//             ++pos;
//             while (pos < len && IsIdentifierChar(line[pos]))
//                 ++pos;
//             std::string_view word = line.substr(start, pos - start);
//             SigTokenType type = IsKeyword(word) ? SigTokenType::Keyword : SigTokenType::Identifier;
//             tokens[token_count++] = Token{type, word};
//             if (type == SigTokenType::Identifier)
//             {
//                 identifier_positions.push_back(IdentPos{token_count - 1});
//             }
//             continue;
//         }

//         // Number literal (simple)
//         if ((c >= '0' && c <= '9'))
//         {
//             size_t start = pos;
//             ++pos;
//             while (pos < len && ((line[pos] >= '0' && line[pos] <= '9') || line[pos] == '.' ||
//                                  line[pos] == 'x' || line[pos] == 'X' ||
//                                  (line[pos] >= 'a' && line[pos] <= 'f') ||
//                                  (line[pos] >= 'A' && line[pos] <= 'F')))
//                 ++pos;
//             tokens[token_count++] = Token{SigTokenType::Number, line.substr(start, pos - start)};
//             continue;
//         }

//         // Punctuation (single char)
//         // List of common C++ punctuation characters in signatures
//         if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ',' ||
//             c == ';' || c == ':' || c == '*' || c == '&' || c == '<' || c == '>' || c == '=' ||
//             c == '+' || c == '-' || c == '/' || c == '%' || c == '^' || c == '!' || c == '?' ||
//             c == '~' || c == '.' || c == '#')
//         {
//             tokens[token_count++] = Token{SigTokenType::Punctuation, line.substr(pos, 1)};
//             ++pos;
//             continue;
//         }

//         // Unknown character: treat as Unknown token of length 1
//         tokens[token_count++] = Token{SigTokenType::Unknown, line.substr(pos, 1)};
//         ++pos;
//     }

//     // Adjust identifier token types for namespaces, classes, and methods based on '::'
//     // Strategy:
//     // - Identify sequences of identifiers separated by '::' punctuation
//     // - Mark all identifiers in such sequences except the last two as Namespace
//     // - Mark the second to last identifier as Class
//     // - Mark the last identifier in the sequence as Method

//     // Collect indices of '::' punctuation tokens
//     std::vector<size_t> scope_resolutions;
//     for (size_t i = 0; i + 1 < token_count; ++i)
//     {
//         if (tokens[i].type == SigTokenType::Punctuation && tokens[i].text == ":" &&
//             tokens[i + 1].type == SigTokenType::Punctuation && tokens[i + 1].text == ":")
//         {
//             scope_resolutions.push_back(i);
//         }
//     }

//     if (!scope_resolutions.empty())
//     {
//         // For each '::', find the identifier before and after it
//         // Then group identifiers separated by '::' into sequences
//         // We'll build sequences of identifier indices separated by '::'
//         std::vector<std::vector<size_t>> sequences;

//         // Build a list of all identifier indices in order
//         std::vector<size_t> id_indices;
//         for (auto& idpos : identifier_positions)
//         {
//             id_indices.push_back(idpos.index);
//         }

//         // Build a map from token index to position in id_indices for quick lookup
//         std::unordered_map<size_t, size_t> tokenIndexToIdPos;
//         for (size_t i = 0; i < id_indices.size(); ++i)
//         {
//             tokenIndexToIdPos[id_indices[i]] = i;
//         }

//         // For each '::' punctuation, get identifiers before and after
//         // We'll mark identifiers connected by '::' as a sequence
//         // We'll iterate through id_indices and group consecutive identifiers separated by '::'
//         std::vector<size_t> current_sequence;
//         for (size_t i = 0; i < id_indices.size(); ++i)
//         {
//             size_t current_token_index = id_indices[i];
//             current_sequence.push_back(current_token_index);

//             // Check if next token is '::' and next identifier follows
//             bool has_next_scope = false;
//             if (i + 1 < id_indices.size())
//             {
//                 size_t next_token_index = id_indices[i + 1];
//                 // Check tokens between current_token_index and next_token_index for '::'
//                 if (next_token_index > current_token_index + 1)
//                 {
//                     if (tokens[current_token_index + 1].type == SigTokenType::Punctuation &&
//                         tokens[current_token_index + 1].text == ":" &&
//                         tokens[current_token_index + 2].type == SigTokenType::Punctuation &&
//                         tokens[current_token_index + 2].text == ":")
//                     {
//                         has_next_scope = true;
//                     }
//                 }
//             }

//             if (!has_next_scope || i + 1 == id_indices.size())
//             {
//                 // End of sequence
//                 sequences.push_back(current_sequence);
//                 current_sequence.clear();
//             }
//         }

//         // Now assign types
//         for (auto& seq : sequences)
//         {
//             if (seq.empty())
//                 continue;
//             // Mark all but last two as Namespace
//             for (size_t i = 0; i + 2 < seq.size(); ++i)
//             {
//                 tokens[seq[i]].type = SigTokenType::Namespace;
//             }
//             if (seq.size() >= 2)
//             {
//                 // Second to last as Class
//                 tokens[seq[seq.size() - 2]].type = SigTokenType::Class;
//             }
//             // Last as Method
//             tokens[seq.back()].type = SigTokenType::Method;
//         }
//     }
//     else
//     {
//         // No '::' found, apply heuristic:
//         // For each Identifier token:
//         // - If immediately followed by '(' punctuation token, mark as Method
//         // - Else if immediately followed by whitespace token or '<' punctuation token, mark as Class
//         for (auto& idpos : identifier_positions)
//         {
//             size_t idx = idpos.index;
//             size_t next_idx = idx + 1;
//             if (next_idx < token_count)
//             {
//                 if (tokens[next_idx].type == SigTokenType::Punctuation &&
//                     tokens[next_idx].text == "(")
//                 {
//                     tokens[idx].type = SigTokenType::Method;
//                 }
//                 else if (
//                     tokens[next_idx].type == SigTokenType::Whitespace ||
//                     (tokens[next_idx].type == SigTokenType::Punctuation && tokens[next_idx].text == "<"))
//                 {
//                     tokens[idx].type = SigTokenType::Class;
//                 }
//             }
//         }
//     }

//     // Removed heuristic marking identifiers preceded by 'class' or 'struct' as Class

//     return token_count;
// }

// ImVec4 GetColorForSigToken(SigTokenType type)
// {
//     // Gruvbox-inspired theme colors
//     // clang-format off
//     switch (type)
//     {
//         case SigTokenType::Keyword:     return ImVec4(0.99f, 0.59f, 0.22f, 1.0f);
//         case SigTokenType::Namespace:   return ImVec4(0.60f, 0.60f, 0.60f, 1.0f);
//         case SigTokenType::Class:       return ImVec4(0.20f, 0.64f, 0.80f, 1.0f);
//         case SigTokenType::Method:      return ImVec4(0.93f, 0.93f, 0.40f, 1.0f);
//         case SigTokenType::Identifier:  return ImVec4(0.90f, 0.86f, 0.79f, 1.0f);
//         case SigTokenType::Punctuation: return ImVec4(0.75f, 0.70f, 0.60f, 1.0f);
//         case SigTokenType::Whitespace:  return ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
//         case SigTokenType::Number:      return ImVec4(0.98f, 0.69f, 0.30f, 1.0f);
//         case SigTokenType::Unknown:     return ImVec4(0.90f, 0.86f, 0.79f, 1.0f);
//         default:                        return ImVec4(0.90f, 0.86f, 0.79f, 1.0f);
//     }
//     // clang-format on
// }

// void RenderCppSignature(std::string_view line)
// {
//     Token tokens[MaxSigTokens];
//     size_t token_count = TokenizeCppSignature(line, tokens);

//     ImGui::BeginGroup();
//     for (size_t i = 0; i < token_count; ++i)
//     {
//         if (i > 0)
//             ImGui::SameLine(0.0f, 0.0f);

//         ImVec4 color = GetColorForSigToken(tokens[i].type);
//         ImGui::PushStyleColor(ImGuiCol_Text, color);
//         ImGui::TextUnformatted(tokens[i].text.data(), tokens[i].text.data() +
//         tokens[i].text.size()); ImGui::PopStyleColor();

//         if (i < token_count - 1)
//             ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x);
//     }
//     ImGui::EndGroup();
// }

// } // namespace CPPRenderer

void RenderLogger()
{
    LOG_SCOPE("::RenderLogger");
    auto& Logger = Graphite::Logger::Logger::Instance();

    ImGui::AlignTextToFramePadding();
    ImGui::Text(ICON_CI_TASKLIST " Levels");
    ImGui::SameLine();
    Graphite::Common::UI::VerticalSeparator();

    ImGui::SameLine();
    ImGui::BeginChild(
        "LevelsScroll",
        ImVec2(0, ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y),
        ImGuiChildFlags_None,
        ImGuiWindowFlags_HorizontalScrollbar);
    auto const levels{Logger.GetLevels()};
    for (std::size_t level_idx = 0; level_idx < levels.size(); ++level_idx)
    {
        auto const& log_level{levels[level_idx]};

        if (level_idx > 0)
        {
            ImGui::SameLine();
        }

        auto const bg_color = log_level.color;
        auto const bg_hover = ImVec4(
            log_level.color.x + 0.2f, log_level.color.y + 0.2f, log_level.color.z + 0.2f, 0.70f);
        ImVec4 bg_active = bg_color;

        ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_color);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bg_hover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, bg_active);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4{0.15f, 0.15f, 0.15f, 1.00f});

        bool enabled{Logger.IsLevelEnabled(log_level.value)};
        if (ImGui::Checkbox(log_level.display.data(), &enabled))
        {
            Logger.SetLevelState(log_level.value, enabled);
        }

        ImGui::PopStyleColor(4);

        if (level_idx + 1 < levels.size())
        {
            ImGui::SameLine();
            Graphite::Common::UI::VerticalSeparator();
        }
    }
    ImGui::EndChild();

    static char filter[128] = ".";
    static std::string s_last_filter = "";
    static std::vector<size_t> s_filtered_indices;
    ImGui::AlignTextToFramePadding();
    ImGui::Text(ICON_CI_SEARCH_SPARKLE " Search");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputTextWithHint("##Search", "Type to filter scopes...", filter, sizeof(filter));

    static std::size_t s_last_scopes_count{0};
    auto const scopes = Logger.GetScopes();
    static std::vector<std::pair<std::string_view, Graphite::Logger::LogScopeFlags>> s_sorted_scopes{};
    {
        s_sorted_scopes.resize(scopes.size());
        size_t scope_idx{0};
        for (auto const& kv : scopes)
        {
            s_sorted_scopes[scope_idx++] = kv;
        }
        std::sort(s_sorted_scopes.begin(), s_sorted_scopes.end(), [](auto const& a, auto const& b) {
            return a.first < b.first;
        });
    }

    // Only recompute the filtered indices when the filter changes
    std::string filter_str(filter);
    if (filter_str != s_last_filter || s_last_scopes_count != scopes.size())
    {
        LOG_TRACE(
            "::RenderLogger(): Computing filtered indices | filter_str != s_last_filter == {} | "
            "s_last_scopes_count != scopes.size() == {}",
            filter_str != s_last_filter,
            s_last_scopes_count != scopes.size());

        s_filtered_indices.clear();
        // Use std::regex for matching, default regex is "." (matches everything)
        try
        {
            std::string regex_pattern = filter_str.empty() ? "." : filter_str;
            std::regex re(regex_pattern, std::regex_constants::icase);
            for (size_t i = 0; i < s_sorted_scopes.size(); ++i)
            {
                auto const& [scope, _] = s_sorted_scopes[i];
                if (std::regex_search(scope.begin(), scope.end(), re))
                {
                    s_filtered_indices.push_back(i);
                }
            }
        }
        catch (const std::regex_error&)
        {
            // If invalid regex, show nothing
            s_filtered_indices.clear();
        }

        s_last_filter = std::move(filter_str);
        s_last_scopes_count = scopes.size();
    }

    if (ImGui::Button(ICON_CI_UNMUTE " Enable All"))
    {
        // Only enable scopes currently filtered and rendered
        for (size_t idx : s_filtered_indices)
        {
            auto const& [scope, _] = s_sorted_scopes[idx];
            Logger.SetScopeEnabled(std::string{scope}, true);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_CI_MUTE " Disable All"))
    {
        // Only disable scopes currently filtered and rendered
        for (size_t idx : s_filtered_indices)
        {
            auto const& [scope, _] = s_sorted_scopes[idx];
            Logger.SetScopeEnabled(std::string{scope}, false);
        }
    }

    ImGui::BeginChild("ScopesList", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (ImGui::BeginTable(
            "ScopesTable",
            2,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn(ICON_CI_TASKLIST " Levels");
        ImGui::TableSetupColumn(ICON_CI_TELESCOPE " Scopes");
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(s_filtered_indices.size()));
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                size_t i = s_filtered_indices[static_cast<std::size_t>(row)];
                auto const& [scope, flags] = s_sorted_scopes[i];

                ImGui::PushID(scope.data());
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                for (std::size_t level_idx = 0; level_idx < levels.size(); ++level_idx)
                {
                    auto const& log_level = levels[level_idx];

                    bool value = flags[log_level.value];

                    auto const bg_color = log_level.color;
                    auto const bg_hover = ImVec4(
                        log_level.color.x + 0.2f,
                        log_level.color.y + 0.2f,
                        log_level.color.z + 0.2f,
                        0.70f);
                    ImVec4 bg_active = bg_color;

                    ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_color);
                    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bg_hover);
                    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, bg_active);
                    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4{0.15f, 0.15f, 0.15f, 1.00f});

                    ImGui::SameLine();
                    ImGui::PushID(static_cast<int>(level_idx));
                    if (ImGui::Checkbox("##level", &value))
                    {
                        Logger.SetScopeLevelEnabled(std::string{scope}, log_level.value, value);
                    }
                    ImGui::PopID();

                    ImGui::PopStyleColor(4);
                }

                ImGui::TableSetColumnIndex(1);
                std::string_view name = scope;

                auto const space = name.find(' ');
                if (space != std::string_view::npos)
                {
                    auto possible_name = name.substr(space + 1);
                    if (possible_name.find('(') != std::string_view::npos)
                    {
                        name = possible_name;
                    }
                }

                auto const first_scope = name.find("::");
                if (first_scope != std::string_view::npos)
                {
                    auto s = name.rfind(' ', first_scope);
                    if (s != std::string_view::npos)
                    {
                        name.remove_prefix(s + 1);
                    }
                }

                auto const open_paren = name.find('(');
                if (open_paren != std::string_view::npos)
                {
                    name = name.substr(0, open_paren);
                }

                // Modules::DevLayer::CPPRenderer::RenderCppSignature(name);
                ImGui::TextUnformatted(name.data());
                // ? Maybe in the future xD
                // if (ImGui::IsItemHovered())
                // {
                //     ImGui::BeginTooltip();
                //     ImGui::PushTextWrapPos(ImGui::GetMainViewport()->Size.x - 40.0f);
                //     // Modules::DevLayer::CPPRenderer::RenderCppSignature(scope);
                //     ImGui::TextUnformatted(scope.data(), scope.data() + scope.size());
                //     ImGui::PopTextWrapPos();
                //     ImGui::EndTooltip();
                // }

                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }

    ImGui::EndChild(); // scollable list
}

} // namespace Fluxion::Application::Layers::Modules::DevLayer
