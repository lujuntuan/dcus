/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2021
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#ifndef DCUS_TRANSFER_H
#define DCUS_TRANSFER_H

#include "dcus/base/define.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

struct DCUS_EXPORT Transfer final {
    CREATE_PUBLIC_PROPERTY(std::string, domain, "")
    CREATE_PUBLIC_PROPERTY(std::string, name, "")
    CREATE_PUBLIC_PROPERTY(float, progress, .0f)
    CREATE_PUBLIC_PROPERTY(uint32_t, total, 0)
    CREATE_PUBLIC_PROPERTY(uint32_t, current, 0)
    CREATE_PUBLIC_PROPERTY(uint32_t, speed, 0)
    CREATE_PUBLIC_PROPERTY(uint32_t, pass, 0)
    CREATE_PUBLIC_PROPERTY(uint32_t, left, 0)
public:
    bool operator==(const Transfer& transfer) const noexcept;
    bool operator!=(const Transfer& transfer) const noexcept;
    bool operator<(const Transfer& transfer) const noexcept;
    bool operator>(const Transfer& transfer) const noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Transfer& transfer) noexcept;
};

class DCUS_EXPORT Transfers : public std::vector<Transfer> {
public:
    ~Transfers();
    Transfer* update(Transfer&& transfer, bool force = false) noexcept;
    void sort() noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Transfers& transfers) noexcept;
};

DCUS_NAMESPACE_END

#endif // DCUS_TRANSFER_H
