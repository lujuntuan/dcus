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

if(ROS_ENABLE)
    set(DCUS_EXAMPLES_LNK_RPC "ros" CACHE STRING "Examples link rpc name")
elseif(VSOMEIP_ENABLE)
    set(DCUS_EXAMPLES_LNK_RPC "vsomeip" CACHE STRING "Examples link rpc name")
elseif(FDBUS_ENABLE)
    set(DCUS_EXAMPLES_LNK_RPC "fdbus" CACHE STRING "Examples link rpc name")
endif()

string(TOLOWER ${DCUS_EXAMPLES_LNK_RPC} DCUS_EXAMPLES_LNK_RPC)

if(${DCUS_EXAMPLES_LNK_RPC} MATCHES "ros")
    message("** Build examples for ${DCUS_EXAMPLES_LNK_RPC}.")
elseif(${DCUS_EXAMPLES_LNK_RPC} MATCHES "vsomeip")
    message("** Build examples for ${DCUS_EXAMPLES_LNK_RPC}.")
elseif(${DCUS_EXAMPLES_LNK_RPC} MATCHES "fdbus")
    message("** Build examples for ${DCUS_EXAMPLES_LNK_RPC}.")
else()
    message(FATAL_ERROR "** Can not find rpc library(name:${DCUS_EXAMPLES_LNK_RPC}) !")
endif()

macro(add_example_library)
    target_link_libraries(
        ${PROJECT_NAME}
        dcus_client_${DCUS_EXAMPLES_LNK_RPC}
        )
endmacro(add_example_library)

add_subdirectory(client_simple)
if(QT5WIDGETS_ENABLE)
    add_subdirectory(client_viewer)
endif()
