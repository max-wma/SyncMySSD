#include "app_ui.h"
#include "theme.h"
#include "imgui.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shlobj.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <fstream>

namespace ui {

AppUI::AppUI() {
    loadConfig();
}

// ─────────────────────────────────────────────────────────────────────────────
// Main render loop
// ─────────────────────────────────────────────────────────────────────────────
void AppUI::render() {
    // Poll async scan result
    if (isScanning_ && scanFuture_.valid() &&
        scanFuture_.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        diffs_      = scanFuture_.get();
        isScanning_ = false;
        countAdded_ = countModified_ = countDeleted_ = 0;
        for (const auto& d : diffs_) {
            switch (d.changeType) {
                case sync::ChangeType::Added:    countAdded_++;    break;
                case sync::ChangeType::Modified: countModified_++; break;
                case sync::ChangeType::Deleted:  countDeleted_++;  break;
            }
        }
    }

    // Poll async sync result
    if (showProgress_ && syncFuture_.valid() &&
        syncFuture_.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        syncFuture_.get();
    }

    // Full-screen ImGui window
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove      | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("##Main", nullptr, flags);
    renderTitleBar();
    ImGui::Spacing();
    renderConfigPanel();
    ImGui::Spacing();
    renderDiffTable();
    renderActionBar();
    ImGui::End();

    if (showConfirmModal_) renderSyncConfirmModal();
    if (showProgress_)     renderProgressOverlay();
}

// ─────────────────────────────────────────────────────────────────────────────
// Title bar
// ─────────────────────────────────────────────────────────────────────────────
void AppUI::renderTitleBar() {
    ImVec4 accent(colors::accent[0], colors::accent[1], colors::accent[2], 1);
    ImGui::TextColored(accent, "  SyncMySSD");
    ImGui::SameLine();
    ImGui::TextDisabled("v1.0  |  File Synchronization Tool");
    ImGui::Separator();
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration panel  (Source / Direction / Destination)
// ─────────────────────────────────────────────────────────────────────────────
void AppUI::renderConfigPanel() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::BeginChild("##Config", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, flags);

    float avail = ImGui::GetContentRegionAvail().x;
    float labelWidth = 110.0f; // Exactly identical for both Source & Dest
    float boxHeight = ImGui::GetFrameHeight(); // Match the exact height of the text input
    float btnWidth = 80.0f;
    float inputWidth = avail - labelWidth - btnWidth - ImGui::GetStyle().ItemSpacing.x * 2.0f;

    ImU32 accentCol = ImGui::ColorConvertFloat4ToU32(ImVec4(colors::accent[0], colors::accent[1], colors::accent[2], 1.0f));

    // Helper to draw a sleek bordered label with perfectly centered text
    auto drawOutlinedLabel = [&](const char* text, const ImVec2& size) {
        ImVec2 p_min = ImGui::GetCursorScreenPos();
        ImVec2 p_max = ImVec2(p_min.x + size.x, p_min.y + size.y);
        
        // Draw 1.5px outline
        ImGui::GetWindowDrawList()->AddRect(p_min, p_max, accentCol, ImGui::GetStyle().FrameRounding, 0, 1.5f);
        
        // Draw centered text
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 textPos = ImVec2(p_min.x + (size.x - textSize.x) * 0.5f, p_min.y + (size.y - textSize.y) * 0.5f);
        ImGui::GetWindowDrawList()->AddText(textPos, accentCol, text);
        
        ImGui::Dummy(size); // Allocate ImGui layout space
    };

    // ── Source ────────────────────────────────────────────────────────────────
    ImGui::Dummy(ImVec2(0, 5.0f)); // Top spacing
    
    drawOutlinedLabel("Source", ImVec2(labelWidth, boxHeight));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputText("##src", sourcePath_, sizeof(sourcePath_), ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Browse##s", ImVec2(btnWidth, 0))) {
        auto p = openFolderDialog();
        if (!p.empty()) {
            strncpy_s(sourcePath_, p.c_str(), sizeof(sourcePath_) - 1);
            saveConfig();
        }
    }

    // ── Destination ──────────────────────────────────────────────────────────
    ImGui::Dummy(ImVec2(0, 5.0f)); // Equal spacing between rows
    
    drawOutlinedLabel("Destination", ImVec2(labelWidth, boxHeight));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputText("##dst", destPath_, sizeof(destPath_), ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Browse##d", ImVec2(btnWidth, 0))) {
        auto p = openFolderDialog();
        if (!p.empty()) {
            strncpy_s(destPath_, p.c_str(), sizeof(destPath_) - 1);
            saveConfig();
        }
    }

    // ── Direction toggle ─────────────────────────────────────────────────────
    ImGui::Dummy(ImVec2(0, 5.0f)); // Equal spacing between rows
    
    float tBtnW = 140.0f; // Width for longer text
    float totalW = tBtnW * 3 + ImGui::GetStyle().ItemSpacing.x * 2.0f;
    ImGui::SetCursorPosX((avail - totalW) * 0.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));

    auto dirBtn = [&](int id, const char* label) {
        bool sel = (syncDirection_ == id);
        if (sel)  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(colors::accent[0], colors::accent[1], colors::accent[2], 0.8f));
        else      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(colors::bgLight[0], colors::bgLight[1], colors::bgLight[2], 1));
        if (ImGui::Button(label, ImVec2(tBtnW, 0))) syncDirection_ = id;
        ImGui::PopStyleColor();
    };
    dirBtn(0, "Source -> Dest"); ImGui::SameLine();
    dirBtn(1, "Dest -> Source"); ImGui::SameLine();
    dirBtn(2, "Sync Both Ways");
    ImGui::PopStyleVar();

    ImGui::Dummy(ImVec2(0, 5.0f)); // Equal bottom spacing
    ImGui::EndChild();
}

// ─────────────────────────────────────────────────────────────────────────────
// Diff table
// ─────────────────────────────────────────────────────────────────────────────
void AppUI::renderDiffTable() {
    float footerH = 56.0f;
    ImVec2 sz(0, ImGui::GetContentRegionAvail().y - footerH);
    ImGui::BeginChild("##Diff", sz, ImGuiChildFlags_Borders);

    if (isScanning_) {
        // Scanning animation
        float t = (float)ImGui::GetTime();
        std::string dots(1 + ((int)(t * 3) % 3), '.');
        auto msg = "Scanning" + dots;
        ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.4f);
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(msg.c_str()).x) * 0.5f);
        ImGui::TextColored(ImVec4(colors::accent[0],colors::accent[1],colors::accent[2],1), "%s", msg.c_str());
    } else if (diffs_.empty()) {
        const char* msg = (strlen(sourcePath_) == 0)
            ? "Select source and destination folders, then press Scan"
            : "No differences found  --  folders are in sync!";
        ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.4f);
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(msg).x) * 0.5f);
        ImGui::TextDisabled("%s", msg);
    } else {
        ImGuiTableFlags tf = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                             ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY |
                             ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable("##T", 5, tf)) {
            ImGui::TableSetupColumn("",         ImGuiTableColumnFlags_WidthFixed, 30);
            ImGui::TableSetupColumn("Status",   ImGuiTableColumnFlags_WidthFixed, 90);
            ImGui::TableSetupColumn("File",     ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Size",     ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableSetupColumn("Modified", ImGuiTableColumnFlags_WidthFixed, 160);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < diffs_.size(); ++i) {
                auto& e = diffs_[i];
                ImGui::TableNextRow();

                ImVec4 col; const char* label; const char* icon;
                switch (e.changeType) {
                    case sync::ChangeType::Added:
                        col = ImVec4(colors::added[0],colors::added[1],colors::added[2],1);
                        label = "New"; icon = "+"; break;
                    case sync::ChangeType::Modified:
                        col = ImVec4(colors::modified[0],colors::modified[1],colors::modified[2],1);
                        label = "Modified"; icon = "~"; break;
                    case sync::ChangeType::Deleted:
                        col = ImVec4(colors::deleted[0],colors::deleted[1],colors::deleted[2],1);
                        label = "Deleted"; icon = "-"; break;
                }

                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(static_cast<int>(i));
                ImGui::Checkbox("##c", &e.selected);
                ImGui::PopID();

                ImGui::TableSetColumnIndex(1);
                ImGui::TextColored(col, "%s %s", icon, label);

                ImGui::TableSetColumnIndex(2);
                if (e.isDirectory)
                    ImGui::TextColored(ImVec4(colors::accent[0],colors::accent[1],colors::accent[2],0.8f),
                        "[DIR]  %s", e.relativePath.string().c_str());
                else
                    ImGui::Text("%s", e.relativePath.string().c_str());

                ImGui::TableSetColumnIndex(3);
                if (!e.isDirectory) {
                    uintmax_t sz2 = (e.changeType == sync::ChangeType::Deleted) ? e.sizeDest : e.sizeSource;
                    ImGui::Text("%s", formatFileSize(sz2).c_str());
                }

                ImGui::TableSetColumnIndex(4);
                auto ft = (e.changeType == sync::ChangeType::Deleted) ? e.lastModifiedDest : e.lastModifiedSource;
                ImGui::Text("%s", formatTime(ft).c_str());
            }
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

// ─────────────────────────────────────────────────────────────────────────────
// Action bar   [Scan]   stats   [Select All / None]   [Sync Selected (n)]
// ─────────────────────────────────────────────────────────────────────────────
void AppUI::renderActionBar() {
    ImGui::Spacing();

    bool canScan = strlen(sourcePath_) > 0 && strlen(destPath_) > 0 && !isScanning_;
    if (!canScan) ImGui::BeginDisabled();
    if (ImGui::Button("Scan for Changes", ImVec2(170, 36))) {
        isScanning_ = true;
        diffs_.clear();
        scanFuture_ = engine_.computeDiffAsync(sourcePath_, destPath_);
    }
    if (!canScan) ImGui::EndDisabled();

    // Stats
    ImGui::SameLine();
    if (!diffs_.empty()) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
        ImGui::TextColored(ImVec4(colors::added[0],colors::added[1],colors::added[2],1),
            "%d new", countAdded_); ImGui::SameLine();
        ImGui::TextColored(ImVec4(colors::modified[0],colors::modified[1],colors::modified[2],1),
            "%d modified", countModified_); ImGui::SameLine();
        ImGui::TextColored(ImVec4(colors::deleted[0],colors::deleted[1],colors::deleted[2],1),
            "%d deleted", countDeleted_);
    }

    // Select helpers
    if (!diffs_.empty()) {
        ImGui::SameLine(ImGui::GetWindowWidth() - 400);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(colors::bgLight[0],colors::bgLight[1],colors::bgLight[2],1));
        if (ImGui::Button("Select All", ImVec2(90, 36)))
            for (auto& d : diffs_) d.selected = true;
        ImGui::SameLine();
        if (ImGui::Button("Select None", ImVec2(90, 36)))
            for (auto& d : diffs_) d.selected = false;
        ImGui::PopStyleColor();
    }

    // Sync button
    int selCount = 0;
    for (const auto& d : diffs_) if (d.selected) selCount++;

    ImGui::SameLine(ImGui::GetWindowWidth() - 195);
    bool canSync = selCount > 0 && !isScanning_;
    if (!canSync) ImGui::BeginDisabled();
    char lbl[64]; snprintf(lbl, sizeof(lbl), "Sync Selected (%d)", selCount);
    if (ImGui::Button(lbl, ImVec2(180, 36))) showConfirmModal_ = true;
    if (!canSync) ImGui::EndDisabled();
}

// ─────────────────────────────────────────────────────────────────────────────
// Confirmation modal
// ─────────────────────────────────────────────────────────────────────────────
void AppUI::renderSyncConfirmModal() {
    ImGui::OpenPopup("Confirm Sync");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(460, 210));

    if (ImGui::BeginPopupModal("Confirm Sync", nullptr, ImGuiWindowFlags_NoResize)) {
        int sel = 0; for (const auto& d : diffs_) if (d.selected) sel++;

        ImGui::Text("Are you sure you want to sync %d item(s)?", sel);
        ImGui::Spacing();
        ImGui::TextDisabled("Source:      %s", sourcePath_);
        ImGui::TextDisabled("Destination: %s", destPath_);
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 250);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(colors::bgLight[0],colors::bgLight[1],colors::bgLight[2],1));
        if (ImGui::Button("Cancel", ImVec2(100, 36))) {
            showConfirmModal_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Confirm", ImVec2(100, 36))) {
            showConfirmModal_ = false;
            showProgress_     = true;
            ImGui::CloseCurrentPopup();

            sync::SyncConfig cfg;
            cfg.sourcePath = sourcePath_;
            cfg.destPath   = destPath_;
            syncFuture_ = engine_.executeSyncAsync(diffs_, cfg,
                [this](const sync::SyncProgress& p) {
                    std::lock_guard<std::mutex> lk(progressMutex_);
                    syncProgress_ = p;
                });
        }
        ImGui::EndPopup();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Progress overlay
// ─────────────────────────────────────────────────────────────────────────────
void AppUI::renderProgressOverlay() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 160));

    ImGui::Begin("##Progress", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    sync::SyncProgress p;
    { std::lock_guard<std::mutex> lk(progressMutex_); p = syncProgress_; }

    if (p.isComplete) {
        ImGui::TextColored(ImVec4(colors::added[0],colors::added[1],colors::added[2],1),
            "Sync complete!");
        ImGui::Spacing();
        ImGui::Text("%zu item(s) processed.", p.totalItems);
        if (!p.errorMessage.empty())
            ImGui::TextColored(ImVec4(colors::deleted[0],colors::deleted[1],colors::deleted[2],1),
                "%s", p.errorMessage.c_str());
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(100, 36))) showProgress_ = false;
    } else {
        ImGui::Text("Syncing files...");
        ImGui::Spacing();
        float frac = p.totalItems ? (float)p.processedItems / (float)p.totalItems : 0;
        ImGui::ProgressBar(frac, ImVec2(-1, 24));
        ImGui::TextDisabled("%s", p.currentFile.c_str());
    }
    ImGui::End();
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────
std::string AppUI::openFolderDialog() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    BROWSEINFOA bi = {};
    bi.lpszTitle = "Select Folder";
    bi.ulFlags   = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderA(&bi);
    std::string result;
    if (pidl) {
        char path[MAX_PATH];
        if (SHGetPathFromIDListA(pidl, path)) result = path;
        CoTaskMemFree(pidl);
    }
    CoUninitialize();
    return result;
}

void AppUI::saveConfig() {
    std::ofstream ofs("settings.txt");
    if (ofs.is_open()) {
        ofs << sourcePath_ << "\n";
        ofs << destPath_ << "\n";
    }
}

void AppUI::loadConfig() {
    std::ifstream ifs("settings.txt");
    if (ifs.is_open()) {
        std::string s, d;
        if (std::getline(ifs, s)) strncpy_s(sourcePath_, s.c_str(), sizeof(sourcePath_) - 1);
        if (std::getline(ifs, d)) strncpy_s(destPath_, d.c_str(), sizeof(destPath_) - 1);
    }
}

std::string AppUI::formatFileSize(uintmax_t bytes) {
    const char* units[] = {"B","KB","MB","GB","TB"};
    int u = 0; double sz = (double)bytes;
    while (sz >= 1024.0 && u < 4) { sz /= 1024.0; u++; }
    std::ostringstream o;
    if (u == 0) o << bytes << " B";
    else        o << std::fixed << std::setprecision(1) << sz << " " << units[u];
    return o.str();
}

std::string AppUI::formatTime(std::filesystem::file_time_type ftime) {
    auto sctp = std::chrono::clock_cast<std::chrono::system_clock>(ftime);
    auto tt   = std::chrono::system_clock::to_time_t(sctp);
    struct tm tm; localtime_s(&tm, &tt);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}

} // namespace ui
