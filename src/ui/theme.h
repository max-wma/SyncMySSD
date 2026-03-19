#pragma once

namespace ui {

/// Apply the modern dark theme with teal accents to ImGui.
void applyModernTheme();

namespace colors {
    constexpr float accent[]        = {0.00f, 0.749f, 0.647f, 1.0f};  // #00BFA5
    constexpr float accentHover[]   = {0.00f, 0.847f, 0.745f, 1.0f};
    constexpr float added[]         = {0.298f, 0.686f, 0.314f, 1.0f}; // green
    constexpr float modified[]      = {1.00f, 0.757f, 0.027f, 1.0f};  // amber
    constexpr float deleted[]       = {0.898f, 0.224f, 0.208f, 1.0f}; // red
    constexpr float bgDark[]        = {0.102f, 0.102f, 0.180f, 1.0f}; // #1a1a2e
    constexpr float bgMid[]         = {0.129f, 0.137f, 0.224f, 1.0f}; // #212139
    constexpr float bgLight[]       = {0.173f, 0.184f, 0.278f, 1.0f}; // #2c2f47
    constexpr float textPrimary[]   = {0.933f, 0.933f, 0.957f, 1.0f};
    constexpr float textSecondary[] = {0.600f, 0.608f, 0.678f, 1.0f};
}

} // namespace ui
