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

#ifndef DCUS_UTILS_SYSTEM_H
#define DCUS_UTILS_SYSTEM_H

#include "dcus/define.h"
#include <string>

DCUS_NAMESPACE_BEGIN

namespace Utils {
extern DCUS_EXPORT bool programCheckSingleton(const std::string& programName);
extern DCUS_EXPORT bool programRegisterTerminate(void (*ExitCallBack)(int));
}

DCUS_NAMESPACE_END
#endif // DCUS_UTILS_SYSTEM_H
