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
    -DWIN32_LEAN_AND_MEAN
    -DELPP_THREAD_SAFE
    -DELPP_WINSOCK2
)