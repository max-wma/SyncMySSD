#include "file_scanner.h"

namespace sync {

FileScanner::FileMap FileScanner::scan(const std::filesystem::path& rootPath) {
    FileMap result;
    if (!std::filesystem::exists(rootPath) || !std::filesystem::is_directory(rootPath))
        return result;

    std::error_code ec;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(
             rootPath, std::filesystem::directory_options::skip_permission_denied, ec)) {
        if (ec) { ec.clear(); continue; }

        auto relPath = std::filesystem::relative(entry.path(), rootPath, ec);
        if (ec) { ec.clear(); continue; }

        FileInfo info;
        info.relativePath = relPath;
        info.isDirectory  = entry.is_directory(ec);
        if (!info.isDirectory) {
            info.size = entry.file_size(ec);
            if (ec) { info.size = 0; ec.clear(); }
        }
        info.lastModified = entry.last_write_time(ec);
        if (ec) ec.clear();

        result[relPath] = info;
    }
    return result;
}

} // namespace sync
