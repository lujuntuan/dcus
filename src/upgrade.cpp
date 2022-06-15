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

#include "dcus/upgrade.h"
#include "dcus/utils/string.h"
#include "importlib/json11.hpp"
#include <iostream>

DCUS_NAMESPACE_BEGIN

std::string File::getSizeStr(uint32_t size) noexcept
{
    return size > 1024 ? Utils::doubleToString(size / 1024.0, 2) + " MB" : std::to_string(size) + " KB";
}

bool File::operator==(const File& file) const noexcept
{
    return (m_domain == file.domain()
        && m_name == file.name()
        //&& m_url== file.url()
        && m_md5 == file.md5()
        && m_sha1 == file.sha1()
        && m_sha256 == file.sha256()
        && m_size == file.size());
}

bool File::operator!=(const File& file) const noexcept
{
    return !(*this == file);
}

std::ostream& operator<<(std::ostream& ostream, const File& file) noexcept
{
    ostream << "    {\n";
    if (!file.domain().empty()) {
        ostream << "      [domain]: " << file.domain() << "\n";
    }
    ostream << "      [name]: " << file.name() << "\n";
    ostream << "      [url]: " << file.url() << "\n";
    if (!file.md5().empty()) {
        ostream << "      [md5]: " << file.md5() << "\n";
    }
    if (!file.sha1().empty()) {
        ostream << "      [sha1]: " << file.sha1() << "\n";
    }
    if (!file.sha256().empty()) {
        ostream << "      [sha256]: " << file.sha256() << "\n";
    }
    ostream << "      [size]: " << file.size()
            << "(" + File::getSizeStr((uint32_t)(file.size() / 1024)) + ")"
            << "\n";
    if (!file.web_url().empty()) {
        ostream << "      [web_url]: " << file.web_url() << "\n";
    }
    ostream << "    }\n";
    return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Files& files) noexcept
{
    ostream << "    [files]: \n";
    for (const auto& file : files) {
        ostream << file;
    }
    return ostream;
}

bool Package::operator==(const Package& package) const noexcept
{
    return (m_domain == package.domain()
        && m_part == package.part()
        && m_version == package.version()
        && m_files == package.files());
}

bool Package::operator!=(const Package& package) const noexcept
{
    return !(*this == package);
}

std::ostream& operator<<(std::ostream& ostream, const Package& package) noexcept
{
    ostream << "  {\n";
    ostream << "    [domain]: " << package.domain() << "\n";
    ostream << "    [part]: " << package.part() << "\n";
    ostream << "    [version]: " << package.version() << "\n";
    if (package.meta().empty()) {
        ostream << "    [meta]: EMPTY\n";
    } else {
        ostream << "    [meta]: " << package.meta().toStream() << "\n";
    }
    ostream << package.files();
    ostream << "  }\n";
    return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Packages& packages) noexcept
{
    ostream << "  [packages]: \n";
    for (const auto& package : packages) {
        ostream << package;
    }
    return ostream;
}

std::string Upgrade::getMethodStr(int method) noexcept
{
    switch (method) {
    case Upgrade::MTHD_SKIP:
        return "SKIP";
    case Upgrade::MTHD_ATTEMPT:
        return "ATTEMPT";
    case Upgrade::MTHD_FORCED:
        return "FORCED";
    default:
        return "SKIP";
    }
}

bool Upgrade::operator==(const Upgrade& upgrade) const noexcept
{
    if (m_id != upgrade.id()) {
        return false;
    }
    return (m_id == upgrade.id()
        && m_download == upgrade.download()
        && m_deploy == upgrade.deploy()
        && m_maintenance == upgrade.maintenance()
        && m_packages == upgrade.packages());
}

bool Upgrade::operator!=(const Upgrade& upgrade) const noexcept
{
    return !(*this == upgrade);
}

std::ostream& operator<<(std::ostream& ostream, const Upgrade& upgrade) noexcept
{
    ostream << "{\n";
    ostream << "  [id]: " << upgrade.id() << "\n";
    ostream << "  [download]: " << Upgrade::getMethodStr(upgrade.download()) << "\n";
    ostream << "  [deploy]: " << Upgrade::getMethodStr(upgrade.deploy()) << "\n";
    ostream << "  [maintenance]: " << (upgrade.maintenance() ? "true" : "false") << "\n";
    ostream << upgrade.packages();
    ostream << "}";
    return ostream;
}

DCUS_NAMESPACE_END
