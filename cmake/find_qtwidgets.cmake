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
find_package(QT NAMES Qt6 Qt5 QUIET)
find_package(Qt${QT_VERSION_MAJOR}Widgets QUIET)
if(Qt${QT_VERSION_MAJOR}Widgets_FOUND)
    set(QTWIDGETS_ENABLE ON)
    message(STATUS "Qt${QT_VERSION_MAJOR}Widgets found.")
else()
    set(QTWIDGETS_ENABLE OFF)
    message(STATUS "Qt${QT_VERSION_MAJOR}Widgets not found.")
endif()
