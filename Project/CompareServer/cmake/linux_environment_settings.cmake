######################################################
# @brief    linux environment settings of cmake
# @author   Wite_Chen
# @date     2020/01/11
# 
#######################################################


# Set include directories
set(PROJECT_INCLUDE_DIRECTORIES
    ${PROJECT_INCLUDE_DIRECTORIES}
    "${ENV_RAPIDJSON}/include"
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
)

set(PROJECT_LINK_DIRECTORIES_RELEASE
    ${PROJECT_LINK_DIRECTORIES}
)


# Set link libraries
set(PROJECT_LINK_LIBRARIES_DEBUG
    ${PROJECT_LINK_LIBRARIES}
    pthread
)

set(PROJECT_LINK_LIBRARIES_RELEASE
    ${PROJECT_LINK_LIBRARIES}
    pthread
)


# Set definitions
set(PROJECT_DEFINITIONS
    ${PROJECT_DEFINITIONS}
    -DWITH_NONAMESPACES
    -DELPP_THREAD_SAFE
    -DELPP_WINSOCK2
)

# add compile option
add_compile_options(-std=c++11)
