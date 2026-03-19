#include "theme.h"
#include "imgui.h"

namespace ui {

void applyModernTheme() {
    ImGuiStyle& s = ImGui::GetStyle();

    // ── Rounding ─────────────────────────────────────────────────────────────
    s.WindowRounding    = 8.0f;
    s.ChildRounding     = 6.0f;
    s.FrameRounding     = 6.0f;
    s.PopupRounding     = 6.0f;
    s.ScrollbarRounding = 6.0f;
    s.GrabRounding      = 4.0f;
    s.TabRounding       = 6.0f;

    // ── Spacing ──────────────────────────────────────────────────────────────
    s.WindowPadding    = ImVec2(16, 16);
    s.FramePadding     = ImVec2(12, 8);
    s.ItemSpacing      = ImVec2(12, 8);
    s.ItemInnerSpacing = ImVec2(8, 4);
    s.ScrollbarSize    = 12.0f;

    // ── Borders ──────────────────────────────────────────────────────────────
    s.WindowBorderSize = 0.0f;
    s.ChildBorderSize  = 1.0f;
    s.FrameBorderSize  = 0.0f;
    s.PopupBorderSize  = 1.0f;

    // ── Anti-aliasing ────────────────────────────────────────────────────────
    s.AntiAliasedLines = true;
    s.AntiAliasedFill  = true;

    // ── Colors ───────────────────────────────────────────────────────────────
    ImVec4* c = s.Colors;

    auto v4 = [](const float* f) { return ImVec4(f[0], f[1], f[2], f[3]); };

    // Backgrounds
    c[ImGuiCol_WindowBg]  = v4(colors::bgDark);
    c[ImGuiCol_ChildBg]   = ImVec4(colors::bgMid[0], colors::bgMid[1], colors::bgMid[2], 0.5f);
    c[ImGuiCol_PopupBg]   = ImVec4(colors::bgMid[0], colors::bgMid[1], colors::bgMid[2], 0.95f);

    // Title bar
    c[ImGuiCol_TitleBg]          = v4(colors::bgDark);
    c[ImGuiCol_TitleBgActive]    = v4(colors::bgMid);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(colors::bgDark[0], colors::bgDark[1], colors::bgDark[2], 0.75f);

    // Borders
    c[ImGuiCol_Border]       = ImVec4(colors::textSecondary[0], colors::textSecondary[1], colors::textSecondary[2], 0.15f);
    c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    // Frames (inputs, checkboxes)
    c[ImGuiCol_FrameBg]        = v4(colors::bgLight);
    c[ImGuiCol_FrameBgHovered] = ImVec4(colors::bgLight[0]+0.05f, colors::bgLight[1]+0.05f, colors::bgLight[2]+0.05f, 1);
    c[ImGuiCol_FrameBgActive]  = ImVec4(colors::accent[0], colors::accent[1], colors::accent[2], 0.3f);

    // Headers
    c[ImGuiCol_Header]        = ImVec4(colors::bgLight[0], colors::bgLight[1], colors::bgLight[2], 0.8f);
    c[ImGuiCol_HeaderHovered] = ImVec4(colors::accent[0], colors::accent[1], colors::accent[2], 0.3f);
    c[ImGuiCol_HeaderActive]  = ImVec4(colors::accent[0], colors::accent[1], colors::accent[2], 0.5f);

    // Buttons
    c[ImGuiCol_Button]        = ImVec4(colors::accent[0], colors::accent[1], colors::accent[2], 0.8f);
    c[ImGuiCol_ButtonHovered] = v4(colors::accentHover);
    c[ImGuiCol_ButtonActive]  = v4(colors::accent);
    c[ImGuiCol_CheckMark]     = v4(colors::accent);

    // Sliders / scrollbar
    c[ImGuiCol_SliderGrab]         = v4(colors::accent);
    c[ImGuiCol_SliderGrabActive]   = v4(colors::accentHover);
    c[ImGuiCol_ScrollbarBg]        = ImVec4(colors::bgDark[0], colors::bgDark[1], colors::bgDark[2], 0.5f);
    c[ImGuiCol_ScrollbarGrab]      = ImVec4(colors::textSecondary[0], colors::textSecondary[1], colors::textSecondary[2], 0.3f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(colors::textSecondary[0], colors::textSecondary[1], colors::textSecondary[2], 0.5f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(colors::accent[0], colors::accent[1], colors::accent[2], 0.7f);

    // Separator
    c[ImGuiCol_Separator] = ImVec4(colors::textSecondary[0], colors::textSecondary[1], colors::textSecondary[2], 0.15f);

    // Tabs
    c[ImGuiCol_Tab]                = v4(colors::bgMid);
    c[ImGuiCol_TabHovered]         = ImVec4(colors::accent[0], colors::accent[1], colors::accent[2], 0.4f);
    c[ImGuiCol_TabSelected]        = ImVec4(colors::accent[0], colors::accent[1], colors::accent[2], 0.25f);
    c[ImGuiCol_TabSelectedOverline]= v4(colors::accent);

    // Tables
    c[ImGuiCol_TableHeaderBg]    = v4(colors::bgMid);
    c[ImGuiCol_TableBorderStrong]= ImVec4(colors::textSecondary[0], colors::textSecondary[1], colors::textSecondary[2], 0.15f);
    c[ImGuiCol_TableBorderLight] = ImVec4(colors::textSecondary[0], colors::textSecondary[1], colors::textSecondary[2], 0.08f);
    c[ImGuiCol_TableRowBg]       = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TableRowBgAlt]    = ImVec4(1, 1, 1, 0.02f);

    // Text
    c[ImGuiCol_Text]         = v4(colors::textPrimary);
    c[ImGuiCol_TextDisabled] = v4(colors::textSecondary);

    // Progress bar
    c[ImGuiCol_PlotHistogram]        = v4(colors::accent);
    c[ImGuiCol_PlotHistogramHovered] = v4(colors::accentHover);

    // Modal dimming
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0.6f);
}

} // namespace ui
