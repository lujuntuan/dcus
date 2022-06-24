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

#include "dcus/base/define.h"
#include "dcus/base/variant.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

struct File;
struct Package;
using Files = std::vector<File>;
using Packages = std::vector<Package>;

struct DCUS_EXPORT File final {
    std::string domain;
    std::string name;
    std::string url;
    std::string md5;
    std::string sha1;
    std::string sha256;
    uint64_t size = 0;
    std::string web_url;

public:
    static std::string getSizeStr(uint32_t size) noexcept;
    bool operator==(const File& file) const noexcept;
    bool operator!=(const File& file) const noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const File& file) noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Files& files) noexcept;
};

struct DCUS_EXPORT Package final {
    std::string domain;
    std::string part;
    std::string version;
    VariantMap meta;
    Files files;

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
    std::string id;
    Method download = MTHD_SKIP;
    Method deploy = MTHD_SKIP;
    bool maintenance = false;
    Packages packages;

public:
    static std::string getMethodStr(int method) noexcept;
    bool operator==(const Upgrade& upgrade) const noexcept;
    bool operator!=(const Upgrade& upgrade) const noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Upgrade& upgrade) noexcept;
};

VARIANT_DECLARE_TYPE(File);
VARIANT_DECLARE_TYPE(Package);
VARIANT_DECLARE_TYPE(Upgrade);

DCUS_NAMESPACE_END

#endif // DCUS_UPGRADE_H
