######################################################
# @brief    global settings of cmake
# @author   Wite_Chen
# @date     2020/01/11
# 
#######################################################



# version of 3rd-libraries
set(SOCI_VERSION 4.0.1)         # soci
set(SOCI_LIB_VER 4_0)           # soci library version
set(FFMPEG_VERSION )            # ffmpeg
set(ICE_VERSION 3.7.1)          # ice
set(ICE_LIB_VER 37)             # ice library version
set(OPENCV_VERSION 4.0.0)       # opencv
set(OPENCV_LIB_VER 400)         # opencv
set(CURL_VERSION )              # curl
set(OPENSSL_VERSION )           # openssl


# common settings
set(PLAT_TYPE)
set(FILE_CMAKE_ENV)
if(MSVC)
    IF(CMAKE_CL_64)
        set(PLAT_TYPE win64)
    ELSE(CMAKE_CL_64)
        set(PLAT_TYPE win32)
    ENDIF(CMAKE_CL_64)
    set(FILE_CMAKE_ENV msvc_environment_settings.cmake)
elseif(UNIX)
    set(FILE_CMAKE_ENV linux_environment_settings.cmake)
endif()

# cache the path that store the project
string(REGEX REPLACE "(.*)/(.*)/(.*)" "\\1" PROJECT_STORE_PATH  ${PROJECT_SOURCE_DIR})

#set(ENV_DEVELOP "$ENV{DEV_ENV}")
set(ENV_DEVELOP "$ENV{ENV_DEV}")
set(ENV_RAPIDJSON
    "${ENV_DEVELOP}/rapidjson"
  CACHE
  PATH
    "The path of rapidjson"
  FORCE
)

set(ENV_SOCI
    "${ENV_DEVELOP}/soci-${SOCI_VERSION}"
  CACHE
  PATH
    "The path of soci"
  FORCE
)

set (ENV_FACE_SDK
    "${ENV_DEVELOP}/FaceSdk"
  CACHE
  PATH
    "The path of face sdk"
  FORCE
)

set (ENV_LIBEVENT
    "${ENV_DEVELOP}/libevent"
  CACHE
  PATH
    "The path of libevent"
  FORCE
)

set (ENV_FFMPEG
    "${ENV_DEVELOP}/ffmpeg"
  CACHE
  PATH
    "The path of ffmpeg"
  FORCE
)

set (ENV_ICE
        "${ENV_DEVELOP}/zeroc.ice-${ICE_VERSION}"
    CACHE
  PATH
    "The path of ice"
  FORCE
)

set (ENV_OPENCV
    "${ENV_DEVELOP}/opencv-${OPENCV_VERSION}"
  CACHE
  PATH
    "The path of opencv"
  FORCE
)

set (ENV_CURL
    "${ENV_DEVELOP}/libcurl"
  CACHE
  PATH
    "The path of libcurl"
  FORCE
)

set (ENV_OPENSSL
    "${ENV_DEVELOP}/openssl"
  CACHE
  PATH
    "The path of openssl"
  FORCE
)