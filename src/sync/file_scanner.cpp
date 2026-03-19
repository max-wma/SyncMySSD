#include "file_scanner.h"

namespace sync {

FileScanner::FileMap FileScanner::scan(const std::filesystem::path& rootPath, const std::vector<std::string>& exclusions) {
    FileMap result;
    if (!std::filesystem::exists(rootPath) || !std::filesystem::is_directory(rootPath))
        return result;

    std::error_code ec;
    auto it = std::filesystem::recursive_directory_iterator(rootPath, std::filesystem::directory_options::skip_permission_denied, ec);
    auto end = std::filesystem::recursive_directory_iterator();

    while (it != end) {
        if (ec) { ec.clear(); it.increment(ec); continue; }

        bool isDir = it->is_directory(ec);
        if (ec) { ec.clear(); }

        if (isDir && it.depth() == 0) {
            std::string folderName = it->path().filename().string();
            if (std::find(exclusions.begin(), exclusions.end(), folderName) != exclusions.end()) {
                it.disable_recursion_pending();
                it.increment(ec);
                continue; 
            }
        }

        auto relPath = std::filesystem::relative(it->path(), rootPath, ec);
        if (ec) { ec.clear(); it.increment(ec); continue; }

        FileInfo info;
        info.relativePath = relPath;
        info.isDirectory  = isDir;
        if (!info.isDirectory) {
            info.size = it->file_size(ec);
            if (ec) { info.size = 0; ec.clear(); }
        }
        info.lastModified = it->last_write_time(ec);
        if (ec) ec.clear();

        result[relPath] = info;
        it.increment(ec);
    }
    return result;
}

} // namespace sync
