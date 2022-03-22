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

#include "time.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

DCUS_NAMESPACE_BEGIN

namespace Common {

template <typename RTYPE, typename TTYPE>
inline RTYPE getCurrent_T() noexcept
{
    const auto& now = std::chrono::system_clock::now();
    const auto& duration = std::chrono::duration_cast<TTYPE>(now.time_since_epoch());
    return (RTYPE)duration.count();
}

template <typename RTYPE, typename TTYPE>
inline void sleep_T(RTYPE time) noexcept
{
    std::this_thread::sleep_for(TTYPE(time));
}

std::string getCurrentTimeString(const char* format) noexcept
{
    const auto& now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), format);
    return ss.str();
}

uint32_t getCurrentTimeSecForString(const std::string& time_str, const char* format) noexcept
{
    std::tm tm = {};
    std::stringstream ss(time_str);
    ss >> std::get_time(&tm, format);
    uint32_t ms = (tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec);
    return ms;
}

uint32_t getCurrent() noexcept
{
    return getCurrent_T<uint32_t, std::chrono::seconds>();
}

uint32_t getMilliCurrent() noexcept
{
    return getCurrent_T<uint32_t, std::chrono::milliseconds>();
}

uint64_t getMicroCurrent() noexcept
{
    return getCurrent_T<uint64_t, std::chrono::microseconds>();
}

uint64_t getNanoCurrent() noexcept
{
    return getCurrent_T<uint64_t, std::chrono::nanoseconds>();
}

void sleep(uint32_t s) noexcept
{
    sleep_T<uint32_t, std::chrono::seconds>(s);
}

void sleepMilli(uint32_t milli_s) noexcept
{
    sleep_T<uint32_t, std::chrono::milliseconds>(milli_s);
}

void sleepMicro(uint64_t micro_s) noexcept
{
    sleep_T<uint64_t, std::chrono::microseconds>(micro_s);
}

void sleepNano(uint64_t nano_s) noexcept
{
    sleep_T<uint64_t, std::chrono::nanoseconds>(nano_s);
}

}
DCUS_NAMESPACE_END
