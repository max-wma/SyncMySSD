#pragma once

#include "sync_types.h"
#include "file_scanner.h"
#include <functional>
#include <future>
#include <mutex>

namespace sync {

class SyncEngine {
public:
    using ProgressCallback = std::function<void(const SyncProgress&)>;

    /// Compare source vs. dest and return a list of differences.
    std::vector<DiffEntry> computeDiff(const std::filesystem::path& source,
                                       const std::filesystem::path& dest,
                                       const std::vector<std::string>& exclusions = {});

    /// Copy / delete the selected entries according to config.
    void executeSync(const std::vector<DiffEntry>& entries,
                     const SyncConfig& config,
                     ProgressCallback callback = nullptr);

    /// Async wrappers ─────────────────────────────────────────────────────────
    std::future<std::vector<DiffEntry>> computeDiffAsync(
        const std::filesystem::path& source,
        const std::filesystem::path& dest,
        const std::vector<std::string>& exclusions = {});

    std::future<void> executeSyncAsync(
        const std::vector<DiffEntry>& entries,
        const SyncConfig& config,
        ProgressCallback callback = nullptr);
};

} // namespace sync
