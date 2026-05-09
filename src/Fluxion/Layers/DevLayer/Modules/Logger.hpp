/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Logger.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Logger module of @see DevLayer.hpp.
///

#pragma once

#include <string_view>

#include "imgui.h"

namespace Fluxion::Application::Layers::Modules::DevLayer {

namespace CPPRenderer {

enum class SigTokenType
{
    Keyword,
    Identifier,
    Namespace,
    Class,
    Method,
    Punctuation,
    Whitespace,
    Number,
    Unknown
};

struct Token
{
    SigTokenType type;
    std::string_view text;
};

bool IsKeyword(std::string_view const word) noexcept;
bool IsIdentifierChar(char const c) noexcept;

static constexpr size_t MaxSigTokens = 200;
size_t TokenizeCppSignature(std::string_view line, Token (&tokens)[MaxSigTokens]);
ImVec4 GetColorForSigToken(SigTokenType type);

void RenderCppSignature(std::string_view line);

} // namespace CPPRenderer

void RenderLogger();

} // namespace Fluxion::Application::Layers::Modules::DevLayer
