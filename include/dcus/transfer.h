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
#include "dcus/base/variant.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

struct DCUS_EXPORT Transfer final {
    std::string domain;
    std::string name;
    float progress = 0;
    uint32_t total = 0;
    uint32_t current = 0;
    uint32_t speed = 0;
    uint32_t pass = 0;
    uint32_t left = 0;

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

VARIANT_DECLARE_TYPE(Transfer);
VARIANT_DECLARE_TYPE(Transfers);

DCUS_NAMESPACE_END

#endif // DCUS_TRANSFER_H
