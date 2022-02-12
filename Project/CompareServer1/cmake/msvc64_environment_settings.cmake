######################################################
# @brief    environment of windows
# @author   Wite_Chen
# @date     2020/01/29
# 
#######################################################


# Set include directories
set(PROJECT_INCLUDE_DIRECTORIES
    ${PROJECT_INCLUDE_DIRECTORIES}
    ${PROJECT_SOURCE_DIR}/common
    ${ENV_RAPIDJSON}/include
    ${ENV_LIBEVENT}/include
    ${ENV_FACE_SDK}/include
)

set(PROJECT_INCLUDE_DIRECTORIES_DEBUG
    ${PROJECT_INCLUDE_DIRECTORIES}
)

set(PROJECT_INCLUDE_DIRECTORIES_RELEASE
    ${PROJECT_INCLUDE_DIRECTORIES}
)


# Set link directories
set(PROJECT_LINK_DIRECTORIES_DEBUG
    ${PROJECT_LINK_DIRECTORIES}
    ${ENV_LIBEVENT}/win64/debug/lib
    ${ENV_FACE_SDK}/win64/debug/lib
)

set(PROJECT_LINK_DIRECTORIES_RELEASE
    ${PROJECT_LINK_DIRECTORIES}
    ${ENV_LIBEVENT}/win64/release/lib
    ${ENV_FACE_SDK}/win64/release/lib
)


# Set link libraries
set(PROJECT_LINK_LIBRARIES_DEBUG
    ${PROJECT_LINK_LIBRARIES}
    event.lib
    event_core.lib
    event_extra.lib
    event_openssl.lib
    ws2_32.lib
    FaceSdk.lib
)

set(PROJECT_LINK_LIBRARIES_RELEASE
    ${PROJECT_LINK_LIBRARIES}
    event.lib
    event_core.lib
    event_extra.lib
    event_openssl.lib
    ws2_32.lib
    FaceSdk.lib
)


# Set definitions
set(PROJECT_DEFINITIONS
    ${PROJECT_DEFINITIONS}
    -DUNICODE
    -D_UNICODE
    -D_USRDLL
    -D_WINSOCK_DEPRECATED_NO_WARNINGS
    -D_CRT_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_WARNINGS
    -DWIN32_LEAN_AND_MEAN
    -DELPP_THREAD_SAFE
    -DELPP_WINSOCK2
)


# Set compile options
add_compile_options(/W1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819 /wd4251")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
modify_default_compiler_flags(
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
    CMAKE_EXE_LINKER_FLAGS
    CMAKE_EXE_LINKER_FLAGS_DEBUG
    CMAKE_EXE_LINKER_FLAGS_RELEASE
)


# Set debugger environment
set(DEBUGGER_ENVIRONMENT_DEBUG
    "PATH=$(Path)"
    ${ENV_LIBEVENT}/win64/debug/bin
    ${ENV_FACE_SDK}/win64/debug/bin
)

set(DEBUGGER_ENVIRONMENT_RELEASE
    "PATH=$(Path)"
    ${ENV_LIBEVENT}/win64/release/bin
    ${ENV_FACE_SDK}/win64/debug/bin
)
