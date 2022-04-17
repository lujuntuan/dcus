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

#ifndef DCUS_UTILS_DIR_H
#define DCUS_UTILS_DIR_H

#include "dcus/define.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

namespace Utils {
extern DCUS_EXPORT bool mkPath(const std::string& path);
extern DCUS_EXPORT bool exists(const std::string& path);
extern DCUS_EXPORT bool remove(const std::string& path);
extern DCUS_EXPORT bool removeAll(const std::string& path);
extern DCUS_EXPORT bool isFile(const std::string& path);
extern DCUS_EXPORT bool isDir(const std::string& path);
extern DCUS_EXPORT std::vector<std::string> getFiles(const std::string& path, const std::string& suffix = "");
extern DCUS_EXPORT std::vector<std::string> getAllFiles(const std::string& path, const std::string& suffix = "");
extern DCUS_EXPORT std::vector<std::string> getFilesByName(const std::string& path, bool maxFirst = true);
extern DCUS_EXPORT std::vector<std::string> getFilesByLastDate(const std::string& path, bool newFirst = true);
extern DCUS_EXPORT std::vector<std::string> getDirsByName(const std::string& path, bool maxFirst = true);
extern DCUS_EXPORT std::vector<std::string> getDirsByLastDate(const std::string& path, bool newFirst = true);
extern DCUS_EXPORT std::string getPathFileName(const std::string& path);
extern DCUS_EXPORT std::string getPathBaseName(const std::string& path);
extern DCUS_EXPORT std::string getPathSuffixName(const std::string& path);
extern DCUS_EXPORT std::string getTempDir();
extern DCUS_EXPORT void removeSubOldDirs(const std::string& dir, int keepCount);
}

DCUS_NAMESPACE_END

#endif // DCUS_UTILS_DIR_H
