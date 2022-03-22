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

find_package(Qt5Widgets QUIET)
if(Qt5Widgets_FOUND)
    set(QT5WIDGETS_ENABLE ON)
    message(STATUS "Qt5Widgets found.")
else()
    set(QT5WIDGETS_ENABLE OFF)
    message(STATUS "Qt5Widgets not found.")
endif()