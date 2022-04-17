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

start_sub_project(lib ${RPC_NAME})

add_library(
    ${PROJECT_NAME}
    ${RPC_NAME}_lib.cpp
    )

add_dcus_interfaces()

end_sub_project()
