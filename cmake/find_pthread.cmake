#*********************************************************************************
#  *Copyright(C): Juntuan.Lu 2022
#  *Author:  Juntuan.Lu
#  *Version: 1.0
#  *Date:  2022/04/01
#  *Phone: 15397182986
#  *Description:
#  *Others:
#  *Function List:
#  *History:
#**********************************************************************************

find_package(Threads QUIET)

if(CMAKE_USE_PTHREADS_INIT)
    set(PTHREAD_ENABLE ON)
    message(STATUS "CMAKE_THREAD_LIBS_INIT=${CMAKE_THREAD_LIBS_INIT}")
    message(STATUS "pthread found")
else()
    set(PTHREAD_ENABLE OFF)
    message(STATUS "pthread not found")
endif()
