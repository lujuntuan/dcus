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

#ifndef DCUS_COMMON_HOST_H
#define DCUS_COMMON_HOST_H

#include "dcus/base/define.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

namespace Common {
extern std::string getExePath();
extern std::string getExeDir(const std::string& path = "");
extern std::string getExeName(const std::string& path = "");
extern std::string getIpAddress(const std::string& ethName = "");
extern std::string getHostName();
extern std::string getEnvironment(const std::string& key);
}

DCUS_NAMESPACE_END

#endif // DCUS_COMMON_HOST_H
