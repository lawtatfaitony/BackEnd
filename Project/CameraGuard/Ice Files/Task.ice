// Task.ice
#pragma once
#include <Ice/BuiltinSequences.ice>


module Task
{
    // pair<camera_id, rtsp>
    dictionary<int, string> dicCameraRtsp;

    // pair<person_id, similarity>
    dictionary<long, float> dicCompareResult;

    // pair<camera_id, state>
    dictionary<int, int> dicCameraState;

    sequence<int> seqLibs;
    // task info
    struct TaskInfo
    {
        string strIdetityId;
        int nTaskId;
        string strTaskName;
        seqLibs vecLib;
        dicCameraRtsp mapCamera1;    // in
        dicCameraRtsp mapCamera2;    // out
        int nInterval = 3;  // s
        double nThreshold = 0.8;
        int nTop = 1;
    }
    struct ComparePerson
    {
        int nLibId = 0;
        string strLibName;
        long nPersonId = 0;
        long nPicId = 0;
        double fScore = 0;
        string strName;
        string strPicPath;
        string strCardNo;
        int nSex = 0;
        int nCategory = 0;
    }
    sequence<ComparePerson> seqCompare;
    // task result
    struct TaskResult
    {
        TaskInfo infoTask;
        int nCameraId = 0;
        string strCapturePath;
        string strCaptureTime;
        seqCompare seqPerson;
    }

    // camera state
    struct CameraState
    {
        int nTaskId;
        int nCameraId;
        bool bOpen;

    }
    sequence<CameraState> seqCameraState;
    // interface of task server
    interface TaskServer
    {
        /*
        * @brief: register a client
        * @data:  20200209
        * @update:
        * @para[in]:    strClientId, identity of client
        */
        void RegisterClient(string strClientId);

        /*
        * @brief: start task
        * @data:  20200209
        * @update:
        * @para[in]:    infoTask, info of task
        * @return:      int, success to return 0, or failed
        */
        int StartTask(TaskInfo infoTask);

        /*
        * @brief: start task
        * @data:  20200209
        * @update:
        * @para[in]:    strClientId, id of client
        * @para[in]:    nTaskId, id of task
        * @return:      int, success to return 0, or failed
        */
        int StopTask(string strClientId, int nTaskId);
    }

    // interface of task client
    interface TaskClient
    {
        /*
        * @brief: update the camera's state
        * @data:  20200209
        * @update:
        * @para[in]:    stateCamera, state of camera
        * @return:      int, success to return 0, or failed
        */
        int UpdateCameraState(seqCameraState stateCamera);

        /*
        * @brief: push the result of task
        * @data:  20200209
        * @update:
        * @para[in]:    rstTask, result of task
        * @return:      int, success to return 0, or failed
        */
        int PushTaskResult(TaskResult rstTask);
    }

}