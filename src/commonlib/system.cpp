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

#include "system.h"
#include <csignal>
#ifdef _WIN32
#include <Windows.h>
#else
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

inline int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return (fcntl(fd, F_SETLK, &fl));
}

inline bool proc_is_singleton(const char* proc_name)
{
    int fd;
    char buf[16] = { 0 };
    char filename[100] = { 0 };
    sprintf(filename, "/tmp/%s.pid", proc_name);
    fd = open(filename, O_RDWR | O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if (fd < 0) {
        printf("open file \"%s\" failed!!!\n", filename);
        fflush(stdout);
        return false;
    }
    if (lockfile(fd) == -1) {
        printf("file \"%s\" locked. proc already exit!!!\n", filename);
        fflush(stdout);
        close(fd);
        return false;
    }
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);
    return true;
}
#endif

DCUS_NAMESPACE_BEGIN
namespace Common {

bool programCheckSingleton(const std::string& programName)
{
#ifdef _WIN32
    HANDLE hObject = CreateMutex(NULL, FALSE, programName.c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hObject);
        return false;
    }
#else
    if (!proc_is_singleton(programName.c_str())) {
        return false;
    }
#endif
    return true;
}

bool programRegisterTerminate(void (*ExitCallBack)(int))
{
#ifdef _WIN32
    typedef void (*SignalHandlerPointer)(int);
    SignalHandlerPointer sintHandler = 0;
    sintHandler = signal(SIGINT, ExitCallBack);
    if (sintHandler == SIG_ERR) {
        return false;
    }
    //
    SignalHandlerPointer stermHandler = 0;
    stermHandler = signal(SIGTERM, ExitCallBack);
    if (stermHandler == SIG_ERR) {
        return false;
    }
    //
#ifdef SIGBREAK
    SignalHandlerPointer breakHandler = 0;
    breakHandler = signal(SIGBREAK, ExitCallBack);
    if (breakHandler == SIG_ERR) {
        return false;
    }
#endif
    //
    SignalHandlerPointer abortHandler = 0;
    abortHandler = signal(SIGABRT, ExitCallBack);
    if (abortHandler == SIG_ERR) {
        return false;
    }
    return true;
#else // POSIX
    struct sigaction shup, sint, squit, sterm;
    //
    shup.sa_handler = ExitCallBack;
    sigemptyset(&shup.sa_mask);
    shup.sa_flags = 0;
#ifdef SA_RESTART
    shup.sa_flags |= SA_RESTART;
#endif
    if (sigaction(SIGHUP, &shup, 0)) {
        return false;
    }
    //
    sint.sa_handler = ExitCallBack;
    sigemptyset(&sint.sa_mask);
    sint.sa_flags = 0;
#ifdef SA_RESTART
    sint.sa_flags |= SA_RESTART;
#endif
    if (sigaction(SIGINT, &sint, 0)) {
        return false;
    }
    //
    squit.sa_handler = ExitCallBack;
    sigemptyset(&squit.sa_mask);
    squit.sa_flags = 0;
#ifdef SA_RESTART
    squit.sa_flags |= SA_RESTART;
#endif
    if (sigaction(SIGQUIT, &squit, 0)) {
        return false;
    }
    //
    sterm.sa_handler = ExitCallBack;
    sigemptyset(&sterm.sa_mask);
    sterm.sa_flags = 0;
#ifdef SA_RESTART
    sterm.sa_flags |= SA_RESTART;
#endif
    if (sigaction(SIGTERM, &sterm, 0)) {
        return false;
    }
    return true;
#endif
}

}
DCUS_NAMESPACE_END
