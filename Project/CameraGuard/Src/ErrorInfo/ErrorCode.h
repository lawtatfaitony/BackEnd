/**************************************************************
* @brief:       error of camera-guard
* @date:         20200123
* @update:
* @auth:         Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once



#define CG_OK                           0
#define CG_UNKNOW_ERROR                 1
#define CG_INVALID_JSON                 2
#define CG_INVALID_PARA                 3
#define CG_EXCUTE_SQL_ERROR             4
#define CG_INVALID_SESSION              5
#define CG_NOT_EXIST                    6
#define CG_INVAOKE_TIMEOUT              7
#define CG_INVALID_DATABASE_CONN        8




#define CG_COMPARE_NOT_CONNECT          50
#define CG_INVALID_COMPARE_INSTANCE     51
#define CG_TASK_SERVER_NOT_CONNECT      52
#define CG_INVALID_TASK_SERVER_INSTANCE 53

// user
#define CG_INVALID_USER                 100
#define CG_USER_ALREADY_EXIST           101
#define CG_PASSWORD_ERROR               102

// library
#define CG_LIBRARY_ALREADY_EXIST        200
#define CG_LIBRARY_NOT_EXIST            201
#define CG_ADD_LIBRARY_COMPARE_FAILED   202
#define CG_ADD_LIBRARY_FAILED           203


// camera
#define CG_CAMERA_INVALID_RTSP          300
#define CG_CAMERA_ALREADY_EXIST         301
#define CG_CAMERA_SEARCH_RTSP_ERROR     302

// task
#define CG_TASK_ALREADY_EXIST           400
#define CG_TASK_SERVER_FAILED           401
#define CG_COMPARE_1V1_FAILED           402

// person
#define CG_ADD_PERSON_FAILED            500
#define CG_PERSON_ALREADY_EXIST         501
#define CG_EXTRACT_FAILED               502
#define CG_INVALID_PERSON_PHOTO         503
#define CG_PERSON_CARD_NO_EXIST         504


// business
#define CG_TASK_IS_RUNNING              601





