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

#ifndef DCUS_COMMON_DISPATCH_H
#define DCUS_COMMON_DISPATCH_H

#include "dcus/base/define.h"
#include <functional>
#include <string>

DCUS_NAMESPACE_BEGIN

namespace Common {
extern bool dispatch(const std::string& oldFile, const std::string& newFile, const std::string& patchFile,
    const std::function<bool()>& breakFunction = nullptr, const std::function<void(size_t, size_t)>& progressFunction = nullptr);
}

DCUS_NAMESPACE_END

#endif // DCUS_COMMON_DISPATCH_H
