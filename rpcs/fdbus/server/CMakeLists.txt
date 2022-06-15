#*********************************************************************************
#  *Copyright(C): Juntuan.Lu 2021
#  *Author:  Juntuan.Lu
#  *Version: 1.0
#  *Date:  2022/04/01
#  *Phone: 15397182986
#  *Description:
#  *Others:
#  *Function List:
#  *History:
#**********************************************************************************

begin_sub_project(app ${RPC_NAME})

add_executable(
    ${PROJECT_NAME}
    ${RPC_NAME}.cpp
    )

add_dcus_interfaces()

end_sub_project()
