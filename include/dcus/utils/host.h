/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2022
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#ifndef DCUS_UTILS_HOST_H
#define DCUS_UTILS_HOST_H

#include "dcus/base/define.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

namespace Utils {
extern DCUS_EXPORT std::string getExePath() noexcept;
extern DCUS_EXPORT std::string getExeDir(const std::string& path = "") noexcept;
extern DCUS_EXPORT std::string getExeName(const std::string& path = "") noexcept;
extern DCUS_EXPORT std::string getIpAddress(const std::string& ethName = "") noexcept;
extern DCUS_EXPORT std::string getHostName() noexcept;
extern DCUS_EXPORT std::string getEnvironment(const std::string& key, const std::string& defaultValue = "") noexcept;
extern DCUS_EXPORT bool setEnvironment(const std::string& key, const std::string& value, bool force = true) noexcept;
}

DCUS_NAMESPACE_END

#endif // DCUS_UTILS_HOST_H
