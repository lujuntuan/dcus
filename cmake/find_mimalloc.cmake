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

begin_3rdparty_search(mimalloc)
find_library(MIMALLOC_LIBRARY
    NAMES
    mimalloc
    libmimalloc
    PATHS
    ${3RD_LIB_SEARCH_PATH}
    )
end_3rdparty_search()

if(MIMALLOC_LIBRARY)
    set(MIMALLOC_ENABLE ON)
    message(STATUS "mimalloc found.")
    message(STATUS "MIMALLOC_LIBRARY=${MIMALLOC_LIBRARY}")
else()
    set(MIMALLOC_ENABLE OFF)
    message(STATUS "mimalloc not found.")
endif()
