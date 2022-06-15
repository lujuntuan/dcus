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

#include "dcus/base/setting.h"
#if (defined(DCUS_BUILD_SERVER) && defined(DCUS_USE_PULL_HTTP))
#include "core.h"

DCUS_NAMESPACE_BEGIN

namespace Core {

extern Status httpDownloadCommon(const std::string& dir, const Files& files, const Data& config,
    const BreakFunction& breakFunction,
    const ProgressFunction& progressFunction);

Status pull(const std::string& dir, const Files& files, const Data& config,
    const BreakFunction& breakFunction,
    const ProgressFunction& progressFunction)
{
    return httpDownloadCommon(dir, files, config, breakFunction, progressFunction);
}
}

DCUS_NAMESPACE_END

#endif
