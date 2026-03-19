#include "sync_engine.h"
#include <algorithm>

namespace sync {

// ── Diff computation ─────────────────────────────────────────────────────────
std::vector<DiffEntry> SyncEngine::computeDiff(
    const std::filesystem::path& source,
    const std::filesystem::path& dest,
    const std::vector<std::string>& exclusions)
{
    auto srcFiles  = FileScanner::scan(source, exclusions);
    auto destFiles = FileScanner::scan(dest, exclusions);
    std::vector<DiffEntry> diffs;

    // Files in source but not dest → Added; in both but changed → Modified
    for (const auto& [relPath, srcInfo] : srcFiles) {
        auto it = destFiles.find(relPath);
        if (it == destFiles.end()) {
            DiffEntry e;
            e.relativePath      = relPath;
            e.changeType         = ChangeType::Added;
            e.sizeSource         = srcInfo.size;
            e.lastModifiedSource = srcInfo.lastModified;
            e.isDirectory        = srcInfo.isDirectory;
            diffs.push_back(e);
        } else if (!srcInfo.isDirectory) {
            const auto& dstInfo = it->second;
            if (srcInfo.size != dstInfo.size ||
                srcInfo.lastModified != dstInfo.lastModified) {
                DiffEntry e;
                e.relativePath      = relPath;
                e.changeType         = ChangeType::Modified;
                e.sizeSource         = srcInfo.size;
                e.sizeDest           = dstInfo.size;
                e.lastModifiedSource = srcInfo.lastModified;
                e.lastModifiedDest   = dstInfo.lastModified;
                diffs.push_back(e);
            }
        }
    }

    // Files in dest but not source → Deleted
    for (const auto& [relPath, dstInfo] : destFiles) {
        if (srcFiles.find(relPath) == srcFiles.end()) {
            DiffEntry e;
            e.relativePath    = relPath;
            e.changeType       = ChangeType::Deleted;
            e.sizeDest         = dstInfo.size;
            e.lastModifiedDest = dstInfo.lastModified;
            e.isDirectory      = dstInfo.isDirectory;
            diffs.push_back(e);
        }
    }

    // Sort by change type, then path
    std::sort(diffs.begin(), diffs.end(), [](const DiffEntry& a, const DiffEntry& b) {
        if (a.changeType != b.changeType) return a.changeType < b.changeType;
        return a.relativePath < b.relativePath;
    });
    return diffs;
}

// ── Sync execution ───────────────────────────────────────────────────────────
void SyncEngine::executeSync(
    const std::vector<DiffEntry>& entries,
    const SyncConfig& config,
    ProgressCallback callback)
{
    SyncProgress progress;
    progress.isRunning = true;
    for (const auto& e : entries)
        if (e.selected) progress.totalItems++;

    std::error_code ec;
    for (const auto& entry : entries) {
        if (!entry.selected) continue;

        progress.currentFile = entry.relativePath.string();
        if (callback) callback(progress);

        auto srcPath = config.sourcePath / entry.relativePath;
        auto dstPath = config.destPath   / entry.relativePath;

        switch (entry.changeType) {
        case ChangeType::Added:
        case ChangeType::Modified:
            if (entry.isDirectory) {
                std::filesystem::create_directories(dstPath, ec);
            } else {
                std::filesystem::create_directories(dstPath.parent_path(), ec);
                std::filesystem::copy_file(srcPath, dstPath,
                    std::filesystem::copy_options::overwrite_existing, ec);
            }
            if (ec) {
                progress.errorMessage += ec.message() + " (" +
                    entry.relativePath.string() + ")\n";
                ec.clear();
            }
            break;
        case ChangeType::Deleted:
            if (entry.isDirectory)
                std::filesystem::remove_all(dstPath, ec);
            else
                std::filesystem::remove(dstPath, ec);
            if (ec) ec.clear();
            break;
        }
        progress.processedItems++;
        if (callback) callback(progress);
    }
    progress.isRunning  = false;
    progress.isComplete = true;
    if (callback) callback(progress);
}

// ── Async wrappers ───────────────────────────────────────────────────────────
std::future<std::vector<DiffEntry>> SyncEngine::computeDiffAsync(
    const std::filesystem::path& source,
    const std::filesystem::path& dest,
    const std::vector<std::string>& exclusions)
{
    return std::async(std::launch::async, [this, source, dest, exclusions]() {
        return computeDiff(source, dest, exclusions);
    });
}

std::future<void> SyncEngine::executeSyncAsync(
    const std::vector<DiffEntry>& entries,
    const SyncConfig& config,
    ProgressCallback callback)
{
    return std::async(std::launch::async, [this, entries, config, callback]() {
        executeSync(entries, config, callback);
    });
}

} // namespace sync
