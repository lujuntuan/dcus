/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2021
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2021/04/22
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#include "dir.h"
#include "importlib/fs_std.hpp"
#include <algorithm>

DCUS_NAMESPACE_BEGIN

namespace Common {

template <bool is_file>
std::vector<std::string> _getByName(const std::string& path, bool maxFirst)
{
    std::vector<std::string> dirs;
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (is_file) {
                if (entry.is_regular_file()) {
                    dirs.push_back(entry.path().string());
                }
            } else {
                if (entry.is_directory()) {
                    dirs.push_back(entry.path().string());
                }
            }
        }
        std::sort(dirs.begin(), dirs.end(), [=, &maxFirst](const std::string& lhs, const std::string& rhs) {
            int lhsNum = -1;
            int rhsNum = -1;
            try {
                lhsNum = std::stoi(fs::directory_entry(lhs).path().stem());
            } catch (...) {
                lhsNum = -1;
            }
            try {
                rhsNum = std::stoi(fs::directory_entry(rhs).path().stem());
            } catch (...) {
                rhsNum = -1;
            }
            if (lhsNum > rhsNum) {
                return maxFirst;
            } else {
                return !maxFirst;
            }
        });
        return dirs;
    } catch (...) {
        return dirs;
    }
}

template <bool is_file>
std::vector<std::string> _getByLastDate(const std::string& path, bool newFirst)
{
    std::vector<std::string> dirs;
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (is_file) {
                if (entry.is_regular_file()) {
                    dirs.push_back(entry.path().string());
                }
            } else {
                if (entry.is_directory()) {
                    dirs.push_back(entry.path().string());
                }
            }
        }
        std::sort(dirs.begin(), dirs.end(), [=, &newFirst](const std::string& lhs, const std::string& rhs) {
            if (fs::directory_entry(lhs).last_write_time() > fs::directory_entry(rhs).last_write_time()) {
                return newFirst;
            } else {
                return !newFirst;
            }
        });
        return dirs;
    } catch (...) {
        return dirs;
    }
}

bool mkPath(const std::string& path)
{
    try {
        return fs::create_directories(path);
    } catch (...) {
        return false;
    }
}
bool exists(const std::string& path)
{
    try {
        return fs::exists(path);
    } catch (...) {
        return false;
    }
}

bool remove(const std::string& path)
{
    try {
        return fs::remove(path);
    } catch (...) {
        return false;
    }
}

bool removeAll(const std::string& path)
{
    try {
        return fs::remove_all(path);
    } catch (...) {
        return false;
    }
}

bool isFile(const std::string& path)
{
    try {
        return fs::is_regular_file(path);
    } catch (...) {
        return false;
    }
}

bool isDir(const std::string& path)
{
    try {
        return fs::is_directory(path);
    } catch (...) {
        return false;
    }
}

std::vector<std::string> getFiles(const std::string& path, const std::string& suffix)
{
    std::vector<std::string> files;
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                if (!suffix.empty()) {
                    if (entry.path().extension() == suffix) {
                        files.push_back(entry.path().string());
                    }
                } else {
                    files.push_back(entry.path().string());
                }
            }
        }
        return files;
    } catch (...) {
        return files;
    }
}

std::vector<std::string> getAllFiles(const std::string& path, const std::string& suffix)
{
    std::vector<std::string> files;
    std::function<void(const std::string& dir)> progressAllFiles = [&](const std::string& dir) {
        try {
            for (const auto& entry : fs::directory_iterator(dir)) {
                if (entry.is_directory()) {
                    progressAllFiles(entry.path().string());
                } else if (entry.is_regular_file()) {
                    if (!suffix.empty()) {
                        if (entry.path().extension() == suffix) {
                            files.push_back(entry.path().string());
                        }
                    } else {
                        files.push_back(entry.path().string());
                    }
                }
            }
            return;
        } catch (...) {
            return;
        }
    };
    progressAllFiles(path);
    return files;
}

std::vector<std::string> getFilesByName(const std::string& path, bool maxFirst)
{
    return _getByName<true>(path, maxFirst);
}

std::vector<std::string> getFilesByLastDate(const std::string& path, bool newFirst)
{
    return _getByName<true>(path, newFirst);
}

std::vector<std::string> getDirsByName(const std::string& path, bool maxFirst)
{
    return _getByName<false>(path, maxFirst);
}

std::vector<std::string> getDirsByLastDate(const std::string& path, bool newFirst)
{
    return _getByName<false>(path, newFirst);
}

std::string getPathFileName(const std::string& path)
{
    try {
        return fs::directory_entry(path).path().filename().string();
    } catch (...) {
        return "";
    }
}

std::string getPathBaseName(const std::string& path)
{
    try {
        return fs::directory_entry(path).path().stem().string();
    } catch (...) {
        return "";
    }
}

std::string getPathSuffixName(const std::string& path)
{
    try {
        return fs::directory_entry(path).path().extension().string();
    } catch (...) {
        return "";
    }
}

std::string getTempDir()
{
    try {
        std::string tempDir = fs::temp_directory_path().string();
#ifdef _WIN32
        std::replace(tempDir.begin(), tempDir.end(), '\\', '/');
#endif
        if (tempDir.back() == '/') {
            tempDir.erase(tempDir.end() - 1, tempDir.end());
        }
        return tempDir;
    } catch (...) {
        return "";
    }
}

void removeSubOldDirs(const std::string& dir, int keepCount)
{
    if (!exists(dir)) {
        return;
    }
    const auto dirs = getDirsByName(dir);
    if ((int)dirs.size() > keepCount) {
        int i = 0;
        for (const auto& tdir : dirs) {
            if (i >= keepCount) {
                removeAll(tdir);
            }
            i++;
        }
    }
}

}

DCUS_NAMESPACE_END
