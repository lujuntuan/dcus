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

#include "dispatch.h"
#include "string.h"
#include <cstring>
#include <fstream>

DCUS_NAMESPACE_BEGIN

namespace Common {
bool dispatch(const std::string& oldFile, const std::string& newFile, const std::string& patchFile,
    const std::function<bool()>& breakFunction, const std::function<void(size_t, size_t)>& progressFunction)
{
    DCUS_UNUSED(breakFunction);
    DCUS_UNUSED(progressFunction);
    int reval = system(stringSprintf(R"(bspath %s %s %s)", oldFile, newFile, patchFile).c_str());
    if (reval != 0) {
        return false;
    }
    return true;
}
}

DCUS_NAMESPACE_END
