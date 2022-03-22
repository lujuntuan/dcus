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
find_package(automotive-dlt QUIET)
start_3rdparty_search(dlt)
find_path(DLT_INCLUDE_DIR
    NAMES
    dlt/dlt.h
    dlt/dlt.hpp
    PATHS
    ${3RD_INC_SEARCH_PATH}
    )
find_library(DLT_LIBRARY
    NAMES
    dlt
    libdlt
    PATHS
    ${3RD_LIB_SEARCH_PATH}
    )
end_3rdparty_search()

if(automotive-dlt_FOUND OR (DLT_INCLUDE_DIR AND DLT_LIBRARY))
    set(DLT_ENABLE ON)
    if(DLT_INCLUDE_DIR AND DLT_LIBRARY)
        message(STATUS "DLT_INCLUDE_DIR=${DLT_INCLUDE_DIR}")
        message(STATUS "DLT_LIBRARY=${DLT_LIBRARY}")
    else()
        message(STATUS "")
    endif()
    message(STATUS "automotive-dlt found")
else()
    set(DLT_ENABLE OFF)
    message(STATUS "automotive-dlt not found")
endif()

