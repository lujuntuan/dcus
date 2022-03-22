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

file(GLOB_RECURSE
    DCUS_CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/include/*
    ${CMAKE_SOURCE_DIR}/src/base/*
    ${CMAKE_SOURCE_DIR}/src/importlib/*
    ${CMAKE_SOURCE_DIR}/src/commonlib/*
    ${CMAKE_SOURCE_DIR}/src/corelib/*
    )
if(DCUS_BUILD_MASTER)
    message("** Build master app")
    file(GLOB
        DCUS_MASTER_SOURCES
        ${CMAKE_SOURCE_DIR}/src/master/*
        )
endif()
if(DCUS_BUILD_WORKER)
    message("** Build worker lib")
    file(GLOB
        DCUS_WORKER_SOURCES
        ${CMAKE_SOURCE_DIR}/src/worker/*
        )
endif()
add_library(
    ${CMAKE_PROJECT_NAME}
    ${DCUS_CORE_SOURCES}
    ${DCUS_MASTER_SOURCES}
    ${DCUS_WORKER_SOURCES}
    )
target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${COMMON_INCLUDE_OUTPUT_PATH}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${OPENSSL_INCLUDE_DIR}
    )
target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    ${OPENSSL_SSL_LIBRARY}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    )
target_compile_definitions(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
    ${DCUS_LIBRARY_DEF}
    "-DPROJECT_NAME=\"${CMAKE_PROJECT_NAME}\""
    "-DPROJECT_VERSION=\"${PROJECT_VERSION}\""
    "-DCPPHTTPLIB_OPENSSL_SUPPORT"
    )
set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES
    VERSION
    ${PROJECT_VERSION}
    SOVERSION
    ${PROJECT_VERSION_MAJOR}
    )
if(WIN32)
    target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        ws2_32
        crypt32
        cryptui
        Iphlpapi
        Shell32
        )
else()
    target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        )
endif()
unset(DCUS_CORE_SOURCES)
unset(DCUS_MASTER_SOURCES)
unset(DCUS_WORKER_SOURCES)

##########

if(DLT_ENABLE)
    option(DCUS_DLT_LOG "DCus use dlt log" OFF)
    if(DCUS_DLT_LOG)
        if(automotive-dlt_FOUND)
            target_link_libraries(
                ${CMAKE_PROJECT_NAME}
                $<BUILD_INTERFACE:Genivi::dlt>
                )
        else()
            target_include_directories(
                ${CMAKE_PROJECT_NAME}
                PRIVATE
                ${DLT_INCLUDE_DIR}
                )
            target_link_libraries(
                ${CMAKE_PROJECT_NAME}
                $<BUILD_INTERFACE:${DLT_LIBRARY}>
                )
        endif()
        target_compile_definitions(
            ${CMAKE_PROJECT_NAME}
            PRIVATE
            "-DDCUS_HAS_DLT"
            )
    endif()
endif()

include(CMakePackageConfigHelpers)
configure_file(
    ${CMAKE_SOURCE_DIR}/config.cmake.in
    ${COMMON_CMAKE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}Config.cmake
    IMMEDIATE
    @ONLY
    )
configure_file(
    ${CMAKE_SOURCE_DIR}/config.h.in
    ${COMMON_INCLUDE_OUTPUT_PATH}/dcus/config.h
    )
write_basic_package_version_file(
    ${COMMON_CMAKE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
    VERSION
    ${PACKAGE_VERSION}
    COMPATIBILITY
    AnyNewerVersion
    )

install(
    TARGETS
    ${CMAKE_PROJECT_NAME}
    EXPORT
    ${CMAKE_PROJECT_NAME}Targets
    RUNTIME
    DESTINATION
    bin
    ARCHIVE
    DESTINATION
    lib
    LIBRARY
    DESTINATION
    lib
    INCLUDES
    DESTINATION
    include
    )
install(
    EXPORT
    ${CMAKE_PROJECT_NAME}Targets
    DESTINATION
    lib/cmake/${CMAKE_PROJECT_NAME}
    )
install(
    DIRECTORY
    ${CMAKE_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME}
    DESTINATION
    include
    PATTERN "master" EXCLUDE
    PATTERN "worker" EXCLUDE
    )
if(DCUS_BUILD_MASTER)
    install(
        DIRECTORY
        ${CMAKE_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME}/master
        DESTINATION
        include/${CMAKE_PROJECT_NAME}
        )
endif()
if(DCUS_BUILD_WORKER)
    install(
        DIRECTORY
        ${CMAKE_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME}/worker
        DESTINATION
        include/${CMAKE_PROJECT_NAME}
        )
endif()
install(
    DIRECTORY
    ${CMAKE_SOURCE_DIR}/share/
    DESTINATION
    share/${CMAKE_PROJECT_NAME}
    )
install(
    DIRECTORY
    ${COMMON_CMAKE_OUTPUT_PATH}/
    DESTINATION
    lib/cmake/${CMAKE_PROJECT_NAME}
    )
install(
    DIRECTORY
    ${COMMON_INCLUDE_OUTPUT_PATH}/
    DESTINATION
    include
    )

add_custom_command(
    TARGET
    ${CMAKE_PROJECT_NAME}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/share/ ${COMMON_SHARE_OUTPUT_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/cmake/find_3rdparty.cmake  ${COMMON_CMAKE_OUTPUT_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/cmake/find_dcus.cmake ${COMMON_CMAKE_OUTPUT_PATH}
    )
install_etc(${CMAKE_PROJECT_NAME})
