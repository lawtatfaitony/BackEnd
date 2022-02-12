######################################################
#
# @brief    common seting of cmake
# @author   Wite_Chen
# @date     2020/01/11
# 
#######################################################


# Retrieve add source files.
recursive_add_source_files(SOURCE_LIST
    "Src"
    "conf"
    "cmake"
    "Version"
)

# Set project dependencies
set(PROJECT_DEPENDENCY
    mongoose
    easylogging
)

set(PROJECT_INCLUDE_DIRECTORIES
    ${INCLUDE_PATH}
)

set(PROJECT_DEFINITIONS
    -DMG_ENABLE_HTTP_STREAMING_MULTIPART
)

# Update version of project
modify_project_version()