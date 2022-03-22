#*********************************************************************************
#  *Copyright(C): Juntuan.Lu 2021
#  *Author:  Lu Juntuan
#  *Version: 1.0
#  *Date:  2021/04/22
#  *Phone: 15397182986
#  *Description:
#  *Others:
#  *Function List:
#  *History:
#**********************************************************************************

start_3rdparty_search(tcmalloc)
find_library(TCMALLOC_LIBRARY
    NAMES
    tcmalloc_minimal
    libtcmalloc_minimal
    PATHS
    ${3RD_LIB_SEARCH_PATH}
    )
end_3rdparty_search()

if(TCMALLOC_LIBRARY)
    set(TCMALLOC_ENABLE ON)
    message(STATUS "tcmalloc found.")
    message(STATUS "TCMALLOC_LIBRARY=${TCMALLOC_LIBRARY}")
else()
    set(TCMALLOC_ENABLE OFF)
    message(STATUS "tcmalloc not found.")
endif()
