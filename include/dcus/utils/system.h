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

#ifndef DCUS_UTILS_SYSTEM_H
#define DCUS_UTILS_SYSTEM_H

#include "dcus/base/define.h"
#include <string>

DCUS_NAMESPACE_BEGIN

namespace Utils {
extern DCUS_EXPORT bool programCheckSingleton(const std::string& programName) noexcept;
extern DCUS_EXPORT bool programRegisterTerminate(void (*ExitCallBack)(int)) noexcept;
}

DCUS_NAMESPACE_END
#endif // DCUS_UTILS_SYSTEM_H
