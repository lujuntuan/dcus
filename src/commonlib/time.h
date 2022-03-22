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

#ifndef DCUS_COMMON_TIME_H
#define DCUS_COMMON_TIME_H

#include "dcus/base/define.h"
#include <ctime>
#include <string>

DCUS_NAMESPACE_BEGIN

namespace Common {
extern std::string getCurrentTimeString(const char* format = "%Y-%m-%d %H:%M:%S") noexcept;
extern uint32_t getCurrentTimeSecForString(const std::string& time_str, const char* format = "%Y-%m-%d %H:%M:%S") noexcept;
extern uint32_t getCurrent() noexcept;
extern uint32_t getMilliCurrent() noexcept;
extern uint64_t getMicroCurrent() noexcept;
extern uint64_t getNanoCurrent() noexcept;
extern void sleep(uint32_t s) noexcept;
extern void sleepMilli(uint32_t milli_s) noexcept;
extern void sleepMicro(uint64_t micro_s) noexcept;
extern void sleepNano(uint64_t nano_s) noexcept;
}

DCUS_NAMESPACE_END

#endif // DCUS_COMMON_TIME_H
