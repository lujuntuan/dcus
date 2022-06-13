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

begin_3rdparty_search(tcmalloc)
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
