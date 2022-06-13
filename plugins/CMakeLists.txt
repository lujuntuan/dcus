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

macro(begin_plugin_project)
    get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(PLUGIN_INTERFACE_NAME ${CMAKE_PROJECT_NAME}_interfaces_${PLUGIN_NAME})
endmacro(begin_plugin_project)

macro(end_plugin_project)
endmacro(end_plugin_project)

#######

if(ROS_ENABLE)
    message("** Build ros plugin")
    add_subdirectory(ros)
endif()
if(VSOMEIP_ENABLE)
    message("** Build vsomeip plugin")
    add_subdirectory(vsomeip)
endif()
if(FDBUS_ENABLE)
    message("** Build fdbus plugin")
    add_subdirectory(fdbus)
endif()
