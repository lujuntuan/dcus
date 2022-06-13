#*********************************************************************************
#  *Copyright(C): Juntuan.Lu 2021
#  *Author:  Juntuan.Lu
#  *Version: 1.0
#  *Date:  2021/04/22
#  *Phone: 15397182986
#  *Description:
#  *Others:
#  *Function List:
#  *History:
#**********************************************************************************

#DCUS = Domain Controllers Upgrade Solution
#域控制器升级解决方案

###############
###Project
###############
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build to Release and Debug")
#set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Where install")

###############
###Common
###############
include(cmake/common.cmake)
message("** Build Type=${CMAKE_BUILD_TYPE}")
message("** Install Dir=${CMAKE_INSTALL_PREFIX}")
set(DCUS_VERSION "unknown")
set(DCUS_COMMITID "unknown")
file(STRINGS ${CMAKE_SOURCE_DIR}/version DCUS_VERSION)
get_git_commit(DCUS_COMMITID)
message("** DCUS_VERSION: ${DCUS_VERSION}")
message("** DCUS_COMMITID: ${DCUS_COMMITID}")

project(dcus VERSION ${DCUS_VERSION} LANGUAGES C CXX)

###############
###Required
###############
include(cmake/find_openssl.cmake)
include(cmake/find_pthread.cmake)
if(NOT OPENSSL_ENABLE)
    message(FATAL_ERROR "** Project required openssl")
endif()

###############
###Platform
###############
include(cmake/platform.cmake)

###############
###Condition
###############
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
if(BUILD_SHARED_LIBS)
    message("** Build Shared library")
    set(DCUS_LIBRARY_DEF "-DDCUS_LIBRARY")
    set(DCUS_LIBRARY_APPDEF "")
else()
    message("** Build Static library")
    set(DCUS_LIBRARY_DEF "-DDCUS_LIBRARY_STATIC")
    set(DCUS_LIBRARY_APPDEF "-DDCUS_LIBRARY_STATIC")
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    if(MSVC)
        set(OPENSSL_MSVC_STATIC_RT TRUE)
        foreach(var
                CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
                )
            if(${var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${var} "${${var}}")
            endif()
        endforeach()
    else()
        set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    endif()
endif()

###############
###Malloc
###############
include(cmake/find_3rdparty.cmake)
set(DCUS_MALLOC "" CACHE STRING "Support: tcmalloc jemalloc mimalloc")
string(TOLOWER "${DCUS_MALLOC}" DCUS_MALLOC)
if(${DCUS_MALLOC} MATCHES "tcmalloc")
    message("** Use tcmalloc")
    include(cmake/find_tcmalloc.cmake)
    if(TCMALLOC_ENABLE)
        link_libraries(${TCMALLOC_LIBRARY})
    else()
        message(FATAL_ERROR "** Can not find tcmalloc")
    endif()
elseif(${DCUS_MALLOC} MATCHES "jemalloc")
    message("** Use jemalloc")
    include(cmake/find_jemalloc.cmake)
    if(JEMALLOC_ENABLE)
        link_libraries(${JEMALLOC_LIBRARY})
    else()
        message(FATAL_ERROR "** Can not find jemalloc")
    endif()
elseif(${DCUS_MALLOC} MATCHES "mimalloc")
    message("** Use mimalloc")
    include(cmake/find_mimalloc.cmake)
    if(MIMALLOC_ENABLE)
        link_libraries(${MIMALLOC_LIBRARY})
    else()
        message(FATAL_ERROR "** Can not find mimalloc")
    endif()
endif()

###############
###Src
###############
option(DCUS_BUILD_SERVER "DCus build server app" ON)
option(DCUS_BUILD_CLIENT "DCus build client lib" ON)
option(DCUS_USE_HTTPS "DCus use https" OFF)
if(NOT DCUS_BUILD_SERVER AND NOT DCUS_BUILD_CLIENT)
    message(FATAL_ERROR "** There are no projects to build")
endif()
include(cmake/subproject.cmake)
include(cmake/find_dlt.cmake)
add_subdirectory(src)

###############
###Plugins
###############
include(cmake/find_ros.cmake)
include(cmake/find_vsomeip.cmake)
include(cmake/find_fdbus.cmake)
set(DCUS_BUILD_PLUGINS OFF)
if(ROS_ENABLE OR VSOMEIP_ENABLE OR FDBUS_ENABLE)
    set(DCUS_BUILD_PLUGINS ON)
    message("** Build plugins")
    add_subdirectory(plugins)
endif()

###############
###Examples
###############
if(DCUS_BUILD_PLUGINS AND DCUS_BUILD_CLIENT)
    option(DCUS_BUILD_EXAMPLES "DCus build examples" ON)
    if(DCUS_BUILD_EXAMPLES)
        include(cmake/find_qtwidgets.cmake)
        message("** Build examples")
        add_subdirectory(examples)
    endif()
endif()
