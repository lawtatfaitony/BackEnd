######################################################
#
# @brief    msvc environment settings
# @author   Wite_Chen
# @date     2020/01/14
# 
#######################################################

# set pre-compile options
set(PROJECT_DEFINITIONS
    ${PROJECT_DEFINITIONS}
)

# Set include directories
set(PROJECT_INCLUDE_DIRECTORIES
    ${PROJECT_INCLUDE_DIRECTORIES}
    ${PROJECT_SOURCE_DIR}/common
    ${ENV_OPENSSL}/include
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
)

set(PROJECT_LINK_LIBRARIES_RELEASE
    ${PROJECT_LINK_LIBRARIES}
)

# Set debugger environment
set(DEBUGGER_ENVIRONMENT_DEBUG
    "PATH=$(Path)"
)

set(DEBUGGER_ENVIRONMENT_RELEASE
    "PATH=$(Path)"
)

# set compile option
add_compile_options(-bigobj)