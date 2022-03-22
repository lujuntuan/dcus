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

#include "verify.h"
#include <cstring>
#include <fstream>
#include <functional>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <sstream>

DCUS_NAMESPACE_BEGIN

template <int DIGEST_LENGTH>
inline std::string getHexForDigest(unsigned char* result)
{
    char hex[DIGEST_LENGTH * 2 + 1] = { 0 };
    for (int i = 0; i < DIGEST_LENGTH; i++) {
        sprintf(hex + i * 2, "%02x", result[i]);
    }
    hex[DIGEST_LENGTH * 2] = '\0';
    return std::string(hex);
}

template <int DIGEST_LENGTH>
inline std::string getStrDecrypt(const std::string& srcStr, const std::function<unsigned char*(const unsigned char* d, size_t, unsigned char*)>& decryptFunction)
{
    unsigned char result[DIGEST_LENGTH];
    decryptFunction((const unsigned char*)srcStr.c_str(), srcStr.length(), result);
    return getHexForDigest<DIGEST_LENGTH>(result);
}

template <typename DEC_CTX, int DIGEST_LENGTH>
inline std::string getFileDecrypt(const std::string& filePath,
    const std::function<bool()>& breakFunction,
    const std::function<void(size_t, size_t)>& progressFunction,
    const std::function<int(DEC_CTX*)>& initFunction,
    const std::function<int(DEC_CTX*, const void*, size_t)>& updateFunction,
    const std::function<int(unsigned char*, DEC_CTX*)>& finalFunction)
{
    std::ifstream file(filePath, std::ifstream::in | std::ifstream::binary);
    if (!file.is_open()) {
        return "";
    }
    size_t total = 0;
    if (progressFunction) {
        file.seekg(0, file.end);
        total = file.tellg();
        file.seekg(0, file.beg);
    }
    DEC_CTX ctx;
    initFunction(&ctx);
    char buf[4096u] = { 0 };
    for (; file.good();) {
        if (breakFunction) {
            if (breakFunction() == true) {
                file.close();
                return "";
            }
        }
        if (progressFunction) {
            progressFunction(total, file.tellg());
        }
        file.read(buf, sizeof(buf));
        updateFunction(&ctx, buf, file.gcount());
    }
    file.close();
    unsigned char result[DIGEST_LENGTH];
    finalFunction(result, &ctx);
    if (progressFunction) {
        progressFunction(total, total);
    }
    return getHexForDigest<DIGEST_LENGTH>(result);
}

namespace Common {

std::string getStrMd5(const std::string& srcStr)
{
    return getStrDecrypt<MD5_DIGEST_LENGTH>(srcStr, ::MD5);
}

std::string getStrSha1(const std::string& srcStr)
{
    return getStrDecrypt<SHA_DIGEST_LENGTH>(srcStr, ::SHA1);
}

std::string getStrSha256(const std::string& srcStr)
{
    return getStrDecrypt<SHA256_DIGEST_LENGTH>(srcStr, ::SHA256);
}

std::string getFileMd5(const std::string& filePath, const std::function<bool()>& breakFunction, const std::function<void(size_t, size_t)>& progressFunction)
{
    return getFileDecrypt<MD5_CTX, MD5_DIGEST_LENGTH>(filePath, breakFunction, progressFunction, ::MD5_Init, ::MD5_Update, ::MD5_Final);
}

std::string getFileSha1(const std::string& filePath, const std::function<bool()>& breakFunction, const std::function<void(size_t, size_t)>& progressFunction)
{
    return getFileDecrypt<SHA_CTX, SHA_DIGEST_LENGTH>(filePath, breakFunction, progressFunction, ::SHA1_Init, ::SHA1_Update, ::SHA1_Final);
}

std::string getFileSha256(const std::string& filePath, const std::function<bool()>& breakFunction, const std::function<void(size_t, size_t)>& progressFunction)
{
    return getFileDecrypt<SHA256_CTX, SHA256_DIGEST_LENGTH>(filePath, breakFunction, progressFunction, ::SHA256_Init, ::SHA256_Update, ::SHA256_Final);
}

}

DCUS_NAMESPACE_END
