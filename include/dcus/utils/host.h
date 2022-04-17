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

#ifndef DCUS_UTILS_HOST_H
#define DCUS_UTILS_HOST_H

#include "dcus/define.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

namespace Utils {
extern DCUS_EXPORT std::string getExePath();
extern DCUS_EXPORT std::string getExeDir(const std::string& path = "");
extern DCUS_EXPORT std::string getExeName(const std::string& path = "");
extern DCUS_EXPORT std::string getIpAddress(const std::string& ethName = "");
extern DCUS_EXPORT std::string getHostName();
extern DCUS_EXPORT std::string getEnvironment(const std::string& key);
}

DCUS_NAMESPACE_END

#endif // DCUS_UTILS_HOST_H
