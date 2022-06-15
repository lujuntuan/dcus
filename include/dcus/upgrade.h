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

#ifndef DCUS_UPGRADE_H
#define DCUS_UPGRADE_H

#include "dcus/base/data.h"
#include "dcus/base/define.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

struct File;
struct Package;
using Files = std::vector<File>;
using Packages = std::vector<Package>;

struct DCUS_EXPORT File final {
    CREATE_PUBLIC_PROPERTY(std::string, domain, "")
    CREATE_PUBLIC_PROPERTY(std::string, name, "")
    CREATE_PUBLIC_PROPERTY(std::string, url, "")
    CREATE_PUBLIC_PROPERTY(std::string, md5, "")
    CREATE_PUBLIC_PROPERTY(std::string, sha1, "")
    CREATE_PUBLIC_PROPERTY(std::string, sha256, "")
    CREATE_PUBLIC_PROPERTY(uint64_t, size, 0)
    CREATE_PUBLIC_PROPERTY(std::string, web_url, "") // Additional
public:
    static std::string getSizeStr(uint32_t size) noexcept;
    bool operator==(const File& file) const noexcept;
    bool operator!=(const File& file) const noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const File& file) noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Files& files) noexcept;
};

struct DCUS_EXPORT Package final {
    CREATE_PUBLIC_PROPERTY(std::string, domain, "")
    CREATE_PUBLIC_PROPERTY(std::string, part, "")
    CREATE_PUBLIC_PROPERTY(std::string, version, "")
    CREATE_PUBLIC_PROPERTY(Data, meta, {})
    CREATE_PUBLIC_PROPERTY(Files, files, {})
public:
    bool operator==(const Package& package) const noexcept;
    bool operator!=(const Package& package) const noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Package& package) noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Packages& packages) noexcept;
};

struct DCUS_EXPORT Upgrade final {
    enum Method {
        MTHD_SKIP = 0,
        MTHD_ATTEMPT,
        MTHD_FORCED
    };
    CREATE_PUBLIC_PROPERTY(std::string, id, "")
    CREATE_PUBLIC_PROPERTY(Method, download, MTHD_SKIP)
    CREATE_PUBLIC_PROPERTY(Method, deploy, MTHD_SKIP)
    CREATE_PUBLIC_PROPERTY(bool, maintenance, false)
    CREATE_PUBLIC_PROPERTY(Packages, packages, {})
public:
    static std::string getMethodStr(int method) noexcept;
    bool operator==(const Upgrade& upgrade) const noexcept;
    bool operator!=(const Upgrade& upgrade) const noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Upgrade& upgrade) noexcept;
};

DCUS_NAMESPACE_END

#endif // DCUS_UPGRADE_H
