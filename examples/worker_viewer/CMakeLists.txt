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

start_sub_project(app)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(
    ${PROJECT_NAME}
    "viewer.cpp"
    "mainwindow.cpp"
    "mainwindow.h"
    "mainwindow.ui"
    )

target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Widgets
    )

add_example_library()

end_sub_project()
