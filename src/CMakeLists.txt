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

file(GLOB_RECURSE
    DCUS_SOURCES
    ${CMAKE_SOURCE_DIR}/include/*.*
    ${CMAKE_SOURCE_DIR}/src/*.*
)
list(REMOVE_ITEM DCUS_SOURCES ${CMAKE_SOURCE_DIR}/src/CMakeLists.txt)
if(DCUS_BUILD_SERVER)
    message("** Build server app")
else()
    file(GLOB_RECURSE
        DCUS_SERVER_SOURCES
        ${CMAKE_SOURCE_DIR}/include/dcus/server/*.*
        ${CMAKE_SOURCE_DIR}/src/server/*.*
    )
list(REMOVE_ITEM DCUS_SOURCES ${DCUS_SERVER_SOURCES})
endif()
if(DCUS_BUILD_CLIENT)
    message("** Build client lib")
else()
    file(GLOB_RECURSE
        DCUS_CLIENT_SOURCES
        ${CMAKE_SOURCE_DIR}/include/dcus/client/*.*
        ${CMAKE_SOURCE_DIR}/src/client/*.*
    )
list(REMOVE_ITEM DCUS_SOURCES ${DCUS_CLIENT_SOURCES})
endif()
add_library(
    ${CMAKE_PROJECT_NAME}
    ${DCUS_SOURCES}
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
unset(DCUS_SOURCES)
unset(DCUS_SERVER_SOURCES)
unset(DCUS_CLIENT_SOURCES)

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
    PATTERN "ServerEngine" EXCLUDE
    PATTERN "ClientEngine" EXCLUDE
    PATTERN "server" EXCLUDE
    PATTERN "client" EXCLUDE
)
if(DCUS_BUILD_SERVER)
    install(
        FILES
        ${CMAKE_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME}/ServerEngine
        DESTINATION
        include/${CMAKE_PROJECT_NAME}
    )
    install(
        DIRECTORY
        ${CMAKE_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME}/server
        DESTINATION
        include/${CMAKE_PROJECT_NAME}
    )
endif()
if(DCUS_BUILD_CLIENT)
    install(
        FILES
        ${CMAKE_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME}/ClientEngine
        DESTINATION
        include/${CMAKE_PROJECT_NAME}
    )
    install(
        DIRECTORY
        ${CMAKE_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME}/client
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
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/share/ ${COMMON_SHARE_OUTPUT_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/cmake/find_3rdparty.cmake  ${COMMON_CMAKE_OUTPUT_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/cmake/find_dcus.cmake ${COMMON_CMAKE_OUTPUT_PATH}
)
if(UNIX)
    add_custom_command(
        TARGET
        ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/* ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()
install_etc(${CMAKE_PROJECT_NAME})
