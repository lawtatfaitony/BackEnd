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
    ${ENV_FACE_SDK}/include
    ${ENV_ICE}/build/native/include
    ${ENV_OPENCV}/include
    ${ENV_CURL}/include

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
    ${ENV_LIBEVENT}/${PLAT_TYPE}/debug/lib
    ${ENV_FACE_SDK}/${PLAT_TYPE}/debug/lib
    ${ENV_ICE}/build/native/lib/${PLAT_TYPE}/debug
    ${ENV_OPENCV}/${PLAT_TYPE}/lib
    ${ENV_CURL}/${PLAT_TYPE}/debug/lib

)

set(PROJECT_LINK_DIRECTORIES_RELEASE
    ${PROJECT_LINK_DIRECTORIES}
    ${ENV_LIBEVENT}/${PLAT_TYPE}/release/lib
    ${ENV_FACE_SDK}/${PLAT_TYPE}/release/lib
    ${ENV_ICE}/build/native/lib/${PLAT_TYPE}/release
    ${ENV_OPENCV}/${PLAT_TYPE}/lib
    ${ENV_CURL}/${PLAT_TYPE}/release/lib

)


# Set link libraries
set(PROJECT_LINK_LIBRARIES_DEBUG
    ${PROJECT_LINK_LIBRARIES}
    FaceSdk.lib
    ice${ICE_LIB_VER}d.lib
    ice${ICE_LIB_VER}++11d.lib
    opencv_core${OPENCV_LIB_VER}d.lib
    opencv_imgcodecs${OPENCV_LIB_VER}d.lib
    libcurl_debug.lib

)

set(PROJECT_LINK_LIBRARIES_RELEASE
    ${PROJECT_LINK_LIBRARIES}
    FaceSdk.lib
    ice${ICE_LIB_VER}.lib
    ice${ICE_LIB_VER}++11.lib
    opencv_core${OPENCV_LIB_VER}.lib
    opencv_imgcodecs${OPENCV_LIB_VER}.lib
    libcurl.lib
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
    ${ENV_FACE_SDK}/${PLAT_TYPE}/debug/bin
    ${ENV_ICE}/build/native/bin/${PLAT_TYPE}/debug
    ${ENV_OPENCV}/${PLAT_TYPE}/bin
    ${ENV_CURL}/${PLAT_TYPE}/debug/bin

)

set(DEBUGGER_ENVIRONMENT_RELEASE
    "PATH=$(Path)"
    ${ENV_FACE_SDK}/${PLAT_TYPE}/release/bin
    ${ENV_ICE}/build/native/bin/${PLAT_TYPE}/release
    ${ENV_OPENCV}/${PLAT_TYPE}/bin
    ${ENV_CURL}/${PLAT_TYPE}/release/bin

)
