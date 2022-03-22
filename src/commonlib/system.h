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

#ifndef DCUS_COMMON_SYSTEM_H
#define DCUS_COMMON_SYSTEM_H

#include "dcus/base/define.h"
#include <string>

DCUS_NAMESPACE_BEGIN

namespace Common {
extern bool programCheckSingleton(const std::string& programName);
extern bool programRegisterTerminate(void (*ExitCallBack)(int));
}

DCUS_NAMESPACE_END
#endif // DCUS_COMMON_SYSTEM_H
