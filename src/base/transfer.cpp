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

#include "dcus/base/transfer.h"
#include "commonlib/string.h"
#include "dcus/base/upgrade.h"
#include <algorithm>
#include <iostream>

DCUS_NAMESPACE_BEGIN

bool Transfer::operator==(const Transfer& transfer) const noexcept
{
    return m_domain == transfer.domain()
        && m_name == transfer.name()
        && m_total == transfer.total();
}

bool Transfer::operator!=(const Transfer& transfer) const noexcept
{
    return !(*this == transfer);
}

bool Transfer::operator<(const Transfer& transfer) const noexcept
{
    if (m_total == 0 || transfer.total() == 0) {
        return false;
    }
    if ((double)m_current / m_total > (double)transfer.current() / transfer.total()) {
        return true;
    };
    return false;
}

bool Transfer::operator>(const Transfer& transfer) const noexcept
{
    if (m_total == 0 || transfer.total() == 0) {
        return false;
    }
    if ((double)m_current / m_total < (double)transfer.current() / transfer.total()) {
        return true;
    };
    return false;
}

std::ostream& operator<<(std::ostream& ostream, const Transfer& transfer) noexcept
{
    ostream << "[progress]: " << Common::doubleToString(transfer.progress()) << " %, ";
    if (!transfer.domain().empty()) {
        ostream << "[domain]: " << transfer.domain() << ", ";
    }
    ostream << "[name]: " << transfer.name() << ", ";
    ostream << "[speed]: " << File::getSizeStr(transfer.speed()) << "/S, ";
    ostream << "[total]: " << File::getSizeStr(transfer.total()) << ", ";
    ostream << "[current]: " << File::getSizeStr(transfer.current()) << ", ";
    ostream << "[pass]: " << transfer.pass() << " s, ";
    ostream << "[left]: " << transfer.left() << " s";
    return ostream;
}

Transfer* Transfers::update(Transfer&& transfer, bool force) noexcept
{
    for (Transfer& t : *this) {
        if (t == transfer) {
            t = transfer;
            return &t;
        }
    }
    if (force) {
        push_back(std::move(transfer));
        return &(this->back());
    }
    return nullptr;
}

void Transfers::sort() noexcept
{
    std::sort(begin(), end(), [](const Transfer& lhs, const Transfer& rhs) {
        return lhs < rhs;
    });
}

std::ostream& operator<<(std::ostream& ostream, const Transfers& transfers) noexcept
{
    if (transfers.empty()) {
        return ostream;
    }
    for (const Transfer& transfer : transfers) {
        ostream << transfer << "\n";
    }
    return ostream;
}

DCUS_NAMESPACE_END
