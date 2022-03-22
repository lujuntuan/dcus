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

include(${CMAKE_CURRENT_LIST_DIR}/find_protobuf.cmake)

start_3rdparty_search(fdbus)
find_path(FDBUS_INCLUDE_DIR
    NAMES
    common_base/fdbus.h
    PATHS
    ${3RD_INC_SEARCH_PATH}
    )
find_library(FDBUS_LIBRARY
    NAMES
    common_base
    libcommon_base
    PATHS
    ${3RD_LIB_SEARCH_PATH}
    )
end_3rdparty_search()

if(FDBUS_INCLUDE_DIR AND FDBUS_LIBRARY AND PROTOBUF_ENABLE)
    set(FDBUS_ENABLE ON)
    message(STATUS "fdbus found")
    message(STATUS "FDBUS_INCLUDE_DIR=${FDBUS_INCLUDE_DIR}")
    message(STATUS "FDBUS_LIBRARY=${FDBUS_LIBRARY}")
else()
    set(FDBUS_ENABLE OFF)
    message(STATUS "fdbus not found")
endif()
