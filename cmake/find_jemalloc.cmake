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

begin_3rdparty_search(jemalloc)
find_library(JEMALLOC_LIBRARY
    NAMES
    jemalloc
    libjemalloc
    PATHS
    ${3RD_LIB_SEARCH_PATH}
    )
end_3rdparty_search()

if(JEMALLOC_LIBRARY)
    set(JEMALLOC_ENABLE ON)
    message(STATUS "jemalloc found.")
    message(STATUS "JEMALLOC_LIBRARY=${JEMALLOC_LIBRARY}")
else()
    set(JEMALLOC_ENABLE OFF)
    message(STATUS "jemalloc not found.")
endif()
