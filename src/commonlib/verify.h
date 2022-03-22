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

#ifndef DCUS_COMMON_VERIFY_H
#define DCUS_COMMON_VERIFY_H

#include "dcus/base/define.h"
#include <functional>
#include <string>

DCUS_NAMESPACE_BEGIN

namespace Common {
extern std::string getStrMd5(const std::string& srcStr);
extern std::string getStrSha1(const std::string& srcStr);
extern std::string getStrSha256(const std::string& srcStr);
extern std::string getFileMd5(const std::string& filePath, const std::function<bool()>& breakFunction = nullptr, const std::function<void(size_t, size_t)>& progressFunction = nullptr);
extern std::string getFileSha1(const std::string& filePath, const std::function<bool()>& breakFunction = nullptr, const std::function<void(size_t, size_t)>& progressFunction = nullptr);
extern std::string getFileSha256(const std::string& filePath, const std::function<bool()>& breakFunction = nullptr, const std::function<void(size_t, size_t)>& progressFunction = nullptr);
}

DCUS_NAMESPACE_END

#endif // DCUS_COMMON_DECRYPT_H
