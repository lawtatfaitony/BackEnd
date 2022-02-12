######################################################
# @brief    common seting of cmake
# @author   Wite_Chen
# @date     2020/01/11
# 
#######################################################


# Retrieve add source files.
recursive_add_source_files(SOURCE_LIST
    "Src"
    "conf"
    "model"
    "cmake"
    "Ice Files"
    "generated"
    "Version"
)

# Set project dependencies
set(PROJECT_DEPENDENCY
    mongoose
    easylogging
    Basic
    CameraSearch
    Hanz2Piny
)

# Reset the include directory.
set(PROJECT_INCLUDE_DIRECTORIES
    ${INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

# Update version of project
modify_project_version()