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

#ifndef DCUS_UTILS_VERIFY_H
#define DCUS_UTILS_VERIFY_H

#include "dcus/base/define.h"
#include <functional>
#include <string>

DCUS_NAMESPACE_BEGIN

namespace Utils {
extern DCUS_EXPORT std::string getStrMd5(const std::string& srcStr);
extern DCUS_EXPORT std::string getStrSha1(const std::string& srcStr);
extern DCUS_EXPORT std::string getStrSha256(const std::string& srcStr);
extern DCUS_EXPORT std::string getFileMd5(const std::string& filePath, const std::function<bool()>& breakFunction = nullptr, const std::function<void(size_t, size_t)>& progressFunction = nullptr);
extern DCUS_EXPORT std::string getFileSha1(const std::string& filePath, const std::function<bool()>& breakFunction = nullptr, const std::function<void(size_t, size_t)>& progressFunction = nullptr);
extern DCUS_EXPORT std::string getFileSha256(const std::string& filePath, const std::function<bool()>& breakFunction = nullptr, const std::function<void(size_t, size_t)>& progressFunction = nullptr);
}

DCUS_NAMESPACE_END

#endif // DCUS_UTILS_DECRYPT_H
