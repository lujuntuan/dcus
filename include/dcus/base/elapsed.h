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

#ifndef DCUS_ELAPSED_H
#define DCUS_ELAPSED_H

#include "dcus/base/define.h"
#include <stdint.h>

DCUS_NAMESPACE_BEGIN

class DCUS_EXPORT Elapsed final {
public:
    Elapsed() = default;
    void start(uint32_t milli_s = 0) noexcept;
    void start_nano(uint64_t nano_s = 0) noexcept;
    uint32_t get(uint32_t milli_s = 0) const noexcept;
    uint64_t get_nano(uint64_t nano_s = 0) const noexcept;
    uint32_t restart(uint32_t milli_s = 0) noexcept;
    uint64_t restart_nano(uint64_t nano_s = 0) noexcept;
    void stop() noexcept;
    bool active() const noexcept;
    static uint32_t current() noexcept;
    static uint64_t current_nano() noexcept;

private:
    void start_p(uint64_t nano_s = 0) noexcept;
    uint64_t get_p(uint64_t nano_s = 0) const noexcept;

private:
    uint64_t m_startTime = 0;
};

DCUS_NAMESPACE_END

#endif // DCUS_ELAPSED_H
