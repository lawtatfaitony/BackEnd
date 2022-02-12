######################################################
#
# @brief    common seting of cmake
# @author   Wite_Chen
# @date     2020/12/14
# 
#######################################################


## Retrieve add source files.
recursive_add_source_files(SOURCE_LIST
    "Src"
    "cmake"
)

## Set project dependencies
set(PROJECT_DEPENDENCY
)

## Reset the include directory.
set(PROJECT_INCLUDE_DIRECTORIES
    ${BASIC_INCLUDE_DIRECTORYS}
    ${INCLUDE_PATH}
)
