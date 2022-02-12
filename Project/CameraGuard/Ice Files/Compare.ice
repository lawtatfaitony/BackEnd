// Compare.ice
#pragma once
#include <Ice/BuiltinSequences.ice>


module Compare
{
    struct CompareTask
    {
        string strIdentity;
        int nTaskId = 0;
        int nCameraId = 0;
        double nThreshold = 0.8;
        int nTop = 1;
        string strLibList;
        string strCaptureTime;
        string strPicData;
        string strPicPath;
    }
    struct FaceInfo
    {
        int x;
        int y;
        int nWidth;
        int nHeight;
    }
    dictionary<long, double> dictoryData;
    struct CompareResult
    {
        CompareTask taskInfo;

        // the face info
        FaceInfo infoFace;

        // the path of face
        string strPicPath;

        // store the match result
        dictoryData resultMatch;
    }
    interface CompareServer
    {
        /*
        * @brief: register a client
        * @data:  20200201
        * @update:
        * @para[in]:    strClientId, identity of client
        */
        void RegisterClient(string strClientId);

        /**************************************************************
        *                       library                               *
        **************************************************************/
        /*
        * @brief: add library
        * @data:  20200201
        * @update:
        * @para[in]:    nLibId, id of library
        * @para[out]:   strResult, result of action, empty with failed
            example:
                {
                    "code": 0,
                    "info": 
                    {
                        "lib_id": 1
                    }
                    "msg":""
                }
        * @return:      int, success to return 0, or failed
        */
        int AddLibrary(int nLibId, out string strResult);

        /*
        * @brief: delete library
        * @data:  20200201
        * @update:
        * @para[in]:    nLibId, id of library
        * @return:      int, success to return 0, or failed
        */
        int DeleteLibrary(int nLibId);

        /*
        * @brief: list all library
        * @data:  20200201
        * @update:
        * @para[out]:   strResult, result of action
            example:
                {
                    "code": 0,
                    "info": [
                        {
                            "lib_id": 1
                        },
                        ...
                    ]
                    "msg":""
                }
        * @return:      int, success to return 0, or failed
        */
        int ListLibrary(out string strResult);
    
        /**************************************************************
        *                       person                                *
        **************************************************************/
        /*
        * @brief: add person
        * @data:  20200201
        * @update:
        * @para[in]:    strPicUrl, url of picture
        * @para[out]:   strResult, result of action, empty with failed
            example:
                {
                    "code": 0,
                    "info": 
                    {
                        "person_id": 1
                    }
                    "msg":""
                }
        * @return:      int, success to return 0, or failed
        */
        int AddPerson(int nLibId, string strPicUrl, out string strResult);

        /*
        * @brief: delete person
        * @data:  20200201
        * @update:
        * @para[in]:    nLibId, id of library
        * @para[in]:    nPersonId, id of person
        * @return:      int, success to return 0, or failed
        */
        int DeletePerson(int nLibId, long nPersonId);

        /*
        * @brief: update picture of person
        * @data:  20200207
        * @update:
        * @para[in]:    nLibId, id of library
        * @para[in]:    nPersonId, id of person
        * @para[out]:   strResult, result of action, empty with failed
            example:
                {
                    "code": 0,
                    "info": 
                    {
                        "person_id": 1
                    }
                    "msg":""
                }
        * @return:      int, success to return 0, or failed
        */
        int UpdatePersonPicture(int nLibId, long nPersonId, string strPicUrl, out string strResult);


        /**************************************************************
        *                       compare                               *
        **************************************************************/
        /*
        * @brief: get size of feature
        * @data:  20200201
        * @update:
        * @return:      int, success to result positive vale, or failed
        */
        int GetFeatureSize();

        /*
        * @brief: extract feature of picture
        * @data:  20200201
        * @update:
        * @para[in]:    strPicUrl, url of picture
        * @para[out]:   strResult, result of invoke
        * @return:      int, success to return 0, or failed
        */
        int ExtractFeature(string strPicUrl, out string strResult);

        /*
        * @brief: 1:1 compare
        * @data:  20200201
        * @update:
        * @para[in]:    strLPicUrl, picture of first person
        * @para[in]:    strRPicUrl, picture of second person
        * @para[out]:   nSimilarity,the similarity of two person
        * @return:      int, success to return 0, or failed
        */
        int Compare1v1(string strLPicUrl, string strRPicUrl, out float nSimilarity);

        /*
        * @brief: compare two person with feature
        * @data:  20200704
        * @update:
        * @para[in]:    strLFeature, feature of first person
        * @para[in]:    strRFeature, feature of second person
        * @para[out]:   nSimilarity,the similarity of two person
        * @return:      int, success to return 0, or failed
        */
        int CompareWithFeature(string strLFeature, string strRFeature, out float nSimilarity);

        /*
        * @brief: push compare task
        * @data:  20200201
        * @update:
        * @para[in]:    task, info of task
        * @return:      int, success to return 0, or failed
        */
        int PushCompareTask(CompareTask task);
    };

    interface CompareClient
    {
        /*
        * @brief: push compare result
        * @data:  20200201
        * @update:
        * @para[in]:    result, result of compare task
        * @return:      int, success to return 0, or failed
        */
        int PushCompareResult(CompareResult result);
    };
}