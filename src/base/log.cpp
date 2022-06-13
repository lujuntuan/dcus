/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2022
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#include "dcus/base/log.h"
#include "dcus/base/define.h"
#include <sstream>
#ifdef DCUS_HAS_DLT
#include <dlt/dlt.h>
#endif
#include <mutex>

#define OPCOLO_RESET "\033[0m"
#define OPCOLO_UNDERLINE "\33[4m"
#define OPCOLO_BLACK "\033[30m" /* Black */
#define OPCOLO_RED "\033[31m" /* Red */
#define OPCOLO_GREEN "\033[32m" /* Green */
#define OPCOLO_YELLOW "\033[33m" /* Yellow */
#define OPCOLO_BLUE "\033[34m" /* Blue */
#define OPCOLO_MAGENTA "\033[35m" /* Magenta */
#define OPCOLO_CYAN "\033[36m" /* Cyan */
#define OPCOLO_WHITE "\033[37m" /* White */
#define OPCOLO_BACKGROUND_BLACK "\033[40;37m" /* Background Black */
#define OPCOLO_BACKGROUND_RED "\033[41;37m" /* Background Red */
#define OPCOLO_BACKGROUND_GREEN "\033[42;37m" /* Background Green */
#define OPCOLO_BACKGROUND_YELLOW "\033[43;37m" /* Background Yellow */
#define OPCOLO_BACKGROUND_BLUE "\033[44;37m" /* Background Blue */
#define OPCOLO_BACKGROUND_MAGENTA "\033[45;37m" /* Background Magenta */
#define OPCOLO_BACKGROUND_CYAN "\033[46;37m" /* Background Cyan */
#define OPCOLO_BACKGROUND_WHITE "\033[47;37m" /* Background White */
#define OPCOLO_BOLD_BLACK "\033[1m\033[30m" /* Bold Black */
#define OPCOLO_BOLD_RED "\033[1m\033[31m" /* Bold Red */
#define OPCOLO_BOLD_GREEN "\033[1m\033[32m" /* Bold Green */
#define OPCOLO_BOLD_YELLOW "\033[1m\033[33m" /* Bold Yellow */
#define OPCOLO_BOLD_BLUE "\033[1m\033[34m" /* Bold Blue */
#define OPCOLO_BOLD_MAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define OPCOLO_BOLD_CYAN "\033[1m\033[36m" /* Bold Cyan */
#define OPCOLO_BOLD_WHITE "\033[1m\033[37m" /* Bold White */

std::ostringstream dcusDebugStream_;
std::ostringstream dcusWarningStream_;
std::ostringstream dcusCriticalStream_;
std::ostringstream dcusPropertyStream_;
std::mutex dcusLogMutex_;
#ifdef DCUS_HAS_DLT
DLT_DECLARE_CONTEXT(dltCtx_);
#endif

void dcus_print_initialize(const std::string& name)
{
#ifdef DCUS_HAS_DLT
    DLT_REGISTER_APP(name.c_str(), "Application for Logging");
    DLT_REGISTER_CONTEXT(dltCtx_, "Context", "Context for Logging");
#else
    DCUS_UNUSED(name);
#endif
}

void dcus_print_uninitialize()
{
#ifdef DCUS_HAS_DLT
    DLT_UNREGISTER_CONTEXT(dltCtx_);
    DLT_UNREGISTER_APP();
    dlt_free();
#endif
}

std::ostream& _dcus_print_start(uint8_t type)
{
    dcusLogMutex_.lock();
    switch (type) {
    case DCUS_DEBUG_TYPE_NUM:
        return dcusDebugStream_;
    case DCUS_WARNING_TYPE_NUM:
        std::cerr << OPCOLO_RED;
        return dcusWarningStream_;
    case DCUS_CRITICAL_TYPE_NUM:
        std::cerr << OPCOLO_BOLD_RED;
        return dcusCriticalStream_;
    case DCUS_PROPERTY_TYPE_NUM:
        std::cout << OPCOLO_BOLD_GREEN;
        return dcusPropertyStream_;
    default:
        return dcusDebugStream_;
    }
}

void _dcus_print_final(std::ostream& stream)
{
    std::ostringstream* ss = (std::ostringstream*)&stream;
    const std::string& log = ss->str();
    ss->clear();
    ss->str("");
    if (ss == &dcusDebugStream_) {
        std::cout << log;
        std::cout << std::endl;
#ifdef DCUS_HAS_DLT
        DLT_LOG(dltCtx_, DLT_LOG_DEBUG, DLT_STRING(log.c_str()));
#endif
    } else if (ss == &dcusWarningStream_) {
        std::cerr << log;
        std::cerr << OPCOLO_RESET;
        std::cerr << std::endl;
#ifdef DCUS_HAS_DLT
        DLT_LOG(dltCtx_, DLT_LOG_WARN, DLT_STRING(log.c_str()));
#endif
    } else if (ss == &dcusCriticalStream_) {
        std::cerr << log;
        std::cerr << OPCOLO_RESET;
        std::cerr << std::endl;
        throw std::runtime_error(log);
#ifdef DCUS_HAS_DLT
        DLT_LOG(dltCtx_, DLT_LOG_ERROR, DLT_STRING(log.c_str()));
#endif
    } else if (ss == &dcusPropertyStream_) {
        std::cout << log;
        std::cout << OPCOLO_RESET;
        std::cout << std::endl;
#ifdef DCUS_HAS_DLT
        DLT_LOG(dltCtx_, DLT_LOG_INFO, DLT_STRING(log.c_str()));
#endif
    }
    dcusLogMutex_.unlock();
}
