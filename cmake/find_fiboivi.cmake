#*********************************************************************************
#  *Copyright(C): Juntuan.Lu 2022
#  *Author:  Lu Juntuan
#  *Version: 1.0
#  *Date:  2022/04/01
#  *Phone: 15397182986
#  *Description:
#  *Others:
#  *Function List:
#  *History:
#**********************************************************************************

begin_3rdparty_search(dcus)
find_path(DCUS_INCLUDE_DIR
    NAMES
    dcus/base/application.h
    PATHS
    ${3RD_INC_SEARCH_PATH}
    )
find_library(DCUS_LIBRARY
    NAMES
    dcus
    libdcus
    PATHS
    ${3RD_LIB_SEARCH_PATH}
    )
end_3rdparty_search()

if(DCUS_INCLUDE_DIR AND DCUS_LIBRARY)
    set(DCUS_ENABLE ON)
    message(STATUS "dcus found.")
    message(STATUS "DCUS_INCLUDE_DIR=${DCUS_INCLUDE_DIR}")
    message(STATUS "DCUS_LIBRARY=${DCUS_LIBRARY}")
else()
    set(DCUS_ENABLE OFF)
    message(STATUS "dcus not found.")
endif()
 
