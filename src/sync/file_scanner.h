#pragma once

#include "sync_types.h"
#include <map>

namespace sync {

class FileScanner {
public:
    using FileMap = std::map<std::filesystem::path, FileInfo>;

    /// Recursively scan a directory and return a map of relative-path → FileInfo.
    static FileMap scan(const std::filesystem::path& rootPath, const std::vector<std::string>& exclusions = {});
};

} // namespace sync
