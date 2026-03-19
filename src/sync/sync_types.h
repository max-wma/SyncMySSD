#pragma once

#include <string>
#include <filesystem>
#include <chrono>
#include <vector>

namespace sync {

enum class ChangeType { Added, Modified, Deleted };
enum class SyncDirection { SourceToDest, DestToSource, Bidirectional };

struct FileInfo {
    std::filesystem::path relativePath;
    uintmax_t             size         = 0;
    std::filesystem::file_time_type lastModified{};
    bool                  isDirectory  = false;
};

struct DiffEntry {
    std::filesystem::path relativePath;
    ChangeType            changeType{};
    uintmax_t             sizeSource = 0;
    uintmax_t             sizeDest   = 0;
    std::filesystem::file_time_type lastModifiedSource{};
    std::filesystem::file_time_type lastModifiedDest{};
    bool isDirectory = false;
    bool selected    = true;   // UI checkbox state
};

struct SyncConfig {
    std::filesystem::path sourcePath;
    std::filesystem::path destPath;
    SyncDirection         direction = SyncDirection::SourceToDest;
    std::vector<std::string> excludedFirstLevelFolders;
};

struct SyncProgress {
    size_t      totalItems     = 0;
    size_t      processedItems = 0;
    std::string currentFile;
    bool        isRunning  = false;
    bool        isComplete = false;
    std::string errorMessage;
};

} // namespace sync
