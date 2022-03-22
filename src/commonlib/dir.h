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

#ifndef DCUS_COMMON_DIR_H
#define DCUS_COMMON_DIR_H

#include "dcus/base/define.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

namespace Common {
extern bool mkPath(const std::string& path);
extern bool exists(const std::string& path);
extern bool remove(const std::string& path);
extern bool removeAll(const std::string& path);
extern bool isFile(const std::string& path);
extern bool isDir(const std::string& path);
extern std::vector<std::string> getFiles(const std::string& path, const std::string& suffix = "");
extern std::vector<std::string> getAllFiles(const std::string& path, const std::string& suffix = "");
extern std::vector<std::string> getFilesByName(const std::string& path, bool maxFirst = true);
extern std::vector<std::string> getFilesByLastDate(const std::string& path, bool newFirst = true);
extern std::vector<std::string> getDirsByName(const std::string& path, bool maxFirst = true);
extern std::vector<std::string> getDirsByLastDate(const std::string& path, bool newFirst = true);
extern std::string getPathFileName(const std::string& path);
extern std::string getPathBaseName(const std::string& path);
extern std::string getPathSuffixName(const std::string& path);
extern std::string getTempDir();
extern void removeSubOldDirs(const std::string& dir, int keepCount);
}

DCUS_NAMESPACE_END

#endif // DCUS_COMMON_DIR_H
