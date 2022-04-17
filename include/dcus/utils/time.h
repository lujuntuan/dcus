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

#ifndef DCUS_UTILS_TIME_H
#define DCUS_UTILS_TIME_H

#include "dcus/define.h"
#include <ctime>
#include <string>

DCUS_NAMESPACE_BEGIN

namespace Utils {
extern DCUS_EXPORT std::string getCurrentTimeString(const char* format = "%Y-%m-%d %H:%M:%S") noexcept;
extern DCUS_EXPORT uint32_t getCurrentTimeSecForString(const std::string& time_str, const char* format = "%Y-%m-%d %H:%M:%S") noexcept;
extern DCUS_EXPORT uint32_t getCurrent() noexcept;
extern DCUS_EXPORT uint32_t getMilliCurrent() noexcept;
extern DCUS_EXPORT uint64_t getMicroCurrent() noexcept;
extern DCUS_EXPORT uint64_t getNanoCurrent() noexcept;
extern DCUS_EXPORT void sleep(uint32_t s) noexcept;
extern DCUS_EXPORT void sleepMilli(uint32_t milli_s) noexcept;
extern DCUS_EXPORT void sleepMicro(uint64_t micro_s) noexcept;
extern DCUS_EXPORT void sleepNano(uint64_t nano_s) noexcept;
}

DCUS_NAMESPACE_END

#endif // DCUS_UTILS_TIME_H
