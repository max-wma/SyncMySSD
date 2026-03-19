cmake_minimum_required(VERSION 3.20)
project(SyncMySSD LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Fetch Dear ImGui (docking branch) ──────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# ── ImGui static library ──────────────────────────────────────────────────────
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC d3d11 d3dcompiler dxgi)

# ── Application ───────────────────────────────────────────────────────────────
add_executable(SyncMySSD WIN32
    src/main.cpp
    src/ui/theme.cpp
    src/ui/app_ui.cpp
    src/sync/file_scanner.cpp
    src/sync/sync_engine.cpp
    src/resources/app.rc
    src/resources/app.manifest
)
set_target_properties(SyncMySSD PROPERTIES 
    VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "10.0.17763.0"
)
if(MSVC)
    set_property(TARGET SyncMySSD APPEND_STRING PROPERTY LINK_FLAGS " /MANIFEST:NO") # We use the .rc or direct inclusion
    # Alternatively, use CMake's built-in manifest handling
    target_sources(SyncMySSD PRIVATE src/resources/app.manifest)
endif()
target_include_directories(SyncMySSD PRIVATE src)
target_link_libraries(SyncMySSD PRIVATE imgui_lib dwmapi ole32 shell32)

# Copy the font to the output directory so the executable can find it
add_custom_command(TARGET SyncMySSD POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/Roboto-Regular.ttf"
    "$<TARGET_FILE_DIR:SyncMySSD>"
)
