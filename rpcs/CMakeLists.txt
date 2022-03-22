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

macro(start_rpcs_project)
    get_filename_component(RPC_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(RPC_INTERFACE_NAME ${CMAKE_PROJECT_NAME}_interfaces_${RPC_NAME})
endmacro(start_rpcs_project)

macro(end_rpcs_project)
endmacro(end_rpcs_project)

#######

if(ROS_ENABLE)
    message("** Build ros rpc")
    add_subdirectory(ros)
endif()
if(VSOMEIP_ENABLE)
    message("** Build vsomeip rpc")
    add_subdirectory(vsomeip)
endif()
if(FDBUS_ENABLE)
    message("** Build fdbus rpc")
    add_subdirectory(fdbus)
endif()
