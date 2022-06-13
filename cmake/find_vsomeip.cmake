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

include(${CMAKE_CURRENT_LIST_DIR}/find_commonapi_core.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/find_commonapi_someip.cmake)

find_package(vsomeip QUIET)
find_package(vsomeip3 QUIET)

begin_3rdparty_search(vsomeip)
find_path(VSOMEIP_INCLUDE_DIR
    NAMES
    vsomeip/vsomeip.h
    vsomeip/vsomeip.hpp
    PATHS
    ${3RD_INC_SEARCH_PATH}
    )
find_library(VSOMEIP_LIBRARY
    NAMES
    vsomeip
    libvsomeip
    vsomeip2
    libvsomeip2
    vsomeip3
    libvsomeip3
    PATHS
    ${3RD_LIB_SEARCH_PATH}
    )
end_3rdparty_search()

if(((vsomeip_FOUND OR vsomeip3_FOUND) OR (VSOMEIP_INCLUDE_DIR AND VSOMEIP_LIBRARY)) AND COMMONAPI_CORE_ENABLE AND COMMONAPI_SOMEIP_ENABLE)
    if(COMMONAPI_CORE_GENERATOR_EXE AND COMMONAPI_SOMEIP_GENERATOR_EXE)
        set(VSOMEIP_ENABLE ON)
        if(VSOMEIP_INCLUDE_DIR AND VSOMEIP_LIBRARY)
            message(STATUS "VSOMEIP_INCLUDE_DIR=${VSOMEIP_INCLUDE_DIR}")
            message(STATUS "VSOMEIP_LIBRARY=${VSOMEIP_LIBRARY}")
        else()
            message(STATUS "")
        endif()
    else()
        set(VSOMEIP_ENABLE OFF)
        message(WARNING "COMMONAPI_CORE_GENERATOR or COMMONAPI_SOMEIP_GENERATOR not found")
    endif()
    message(STATUS "vsomeip found")
else()
    set(VSOMEIP_ENABLE OFF)
    message(STATUS "vsomeip not found")
endif()

