#pragma once

#include "sync/sync_engine.h"
#include <string>
#include <vector>
#include <future>
#include <mutex>

namespace ui {

class AppUI {
public:
    AppUI();
    void render();

private:
    // Sub-renders
    void renderTitleBar();
    void renderConfigPanel();
    void renderDiffTable();
    void renderActionBar();
    void renderSyncConfirmModal();
    void renderProgressOverlay();

    // Helpers
    void        saveConfig();
    void        loadConfig();
    std::string openFolderDialog();

    static std::string formatFileSize(uintmax_t bytes);
    static std::string formatTime(std::filesystem::file_time_type time);

    // ── State ────────────────────────────────────────────────────────────────
    char sourcePath_[512] = "";
    char destPath_[512]   = "";
    int  syncDirection_   = 0;   // 0 = →, 1 = ←, 2 = ↔

    std::vector<sync::DiffEntry> diffs_;
    sync::SyncEngine engine_;

    // Async
    std::future<std::vector<sync::DiffEntry>> scanFuture_;
    std::future<void> syncFuture_;
    sync::SyncProgress syncProgress_;
    std::mutex progressMutex_;

    bool isScanning_       = false;
    bool showConfirmModal_ = false;
    bool showProgress_     = false;

    // Stats
    int countAdded_    = 0;
    int countModified_ = 0;
    int countDeleted_  = 0;
};

} // namespace ui
