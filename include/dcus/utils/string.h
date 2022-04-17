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

#ifndef DCUS_UTILS_STRING_H
#define DCUS_UTILS_STRING_H

#include "dcus/define.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

namespace Utils {
extern DCUS_EXPORT void stringReplace(std::string& strBase, const std::string& strSrc, const std::string& strDes) noexcept;
extern DCUS_EXPORT void trimString(std::string& str) noexcept;
extern DCUS_EXPORT int64_t stringToInt64(const std::string& s) noexcept;
extern DCUS_EXPORT std::vector<std::string> stringSplit(const std::string& s, const std::string& delim = ",") noexcept;
extern DCUS_EXPORT int getNumForString(const std::string& numStr, bool* ok = nullptr) noexcept;
extern DCUS_EXPORT bool stringEndWith(const std::string& str, const std::string& tail) noexcept;
extern DCUS_EXPORT bool stringStartWith(const std::string& str, const std::string& head) noexcept;
extern DCUS_EXPORT std::pair<std::string, std::string> getIpaddrMethod(const std::string& url) noexcept;
extern DCUS_EXPORT std::string doubleToString(const double& value, int f = 2) noexcept;
template <typename... Args>
std::string stringSprintf(const std::string& format, Args... args) noexcept
{
    int length = std::snprintf(nullptr, 0, format.c_str(), args.c_str()...);
    if (length <= 0) {
        return "";
    }
    char* buf = new char[length + 1];
    std::snprintf(buf, length + 1, format.c_str(), args.c_str()...);
    std::string str(buf);
    delete[] buf;
    return str;
}
}

DCUS_NAMESPACE_END

#endif // DCUS_UTILS_STRING_H
