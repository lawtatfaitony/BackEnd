#include "LibraryManagement.h"
#include <FaceSdk/FaceSdk.h>
#include <easylogging/easylogging++.h>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <JsonHelper.h>
#include <Macro.h>
#include <Basic/File.h>
#include <Basic/Function.h>
#include <Basic/Stream.h>
#include <Basic/Base64.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Http/LibcurlHelper.h"
#include "../Server/ServerImpl.h"
#include "../ErrorInfo/ErrorMessage.h"


static const std::string kStorageDir = "data/";
static void mat_to_image(const cv::Mat& cvMat, ImageData& imgData)
{
    imgData.width = cvMat.cols;
    imgData.height = cvMat.rows;
    imgData.channels = cvMat.channels();
    imgData.data = cvMat.data;
}

bool download_picture(const std::string& strPicUrl, cv::Mat& cvMat)
{
    HttpPara paraHttp;
    paraHttp.strUrl = strPicUrl;
    LibcurlHelper client;
    std::string strPicData;
    if (CS_OK != client.Get(paraHttp, strPicData))
        return false;
    // byte->mat
    std::vector<char> vecData(strPicData.begin(), strPicData.end());
    cvMat = cv::imdecode(vecData, CV_LOAD_IMAGE_COLOR);
    return true;
}

void convert_task_info(const TaskInfo& infoTask, Compare::CompareTask& task)
{
    task.strIdentity = infoTask.strIdetityId;
    task.nTaskId = infoTask.nTaskId;
    task.nCameraId = infoTask.nCameraId;
    task.nThreshold = infoTask.nThreshold;
    task.nTop = infoTask.nTop;
    task.strLibList = infoTask.strLibList;
    task.strCaptureTime = infoTask.strCaptureTime;
    task.strPicData = infoTask.strPicData;
    task.strPicPath = infoTask.strPicPath;

};

void convert_face_info(const FaceInfo& infoFace, Compare::FaceInfo& face)
{
    face.x = infoFace.x;
    face.y = infoFace.y;
    face.nWidth = infoFace.width;
    face.nHeight = infoFace.height;
};


#define INVOKE_RETURN(func, result) \
    do{ \
        int code = func;    \
        if(code != 0) \
        {   \
            result = ErrorMsgManagement::Instance().GetErrorMsg(code, result);   \
            return 0;   \
        }   \
    }while(false);

LibraryManagement::LibraryManagement()
    : m_pFaceSdk(nullptr)
    , m_nFeatureize(0)
    , m_bStop(false)
    , m_acPerson(1)
{
    m_poolExcuteTask.Start();
    m_thDoTask= std::thread(std::bind(&LibraryManagement::do_task, this));
}

LibraryManagement& LibraryManagement::Instance()
{
    static LibraryManagement g_Instance;
    return g_Instance;
}

LibraryManagement::~LibraryManagement()
{

}

bool LibraryManagement::Init()
{
    FaceSdkPara paraSdk;
    auto nCode = InitFaceSdk(paraSdk);
    if (nCode != kFaceResultOk)
    {
        LOG(ERROR) << "Init facesdk failed:" << nCode;
        return false;
    }
    nCode = CreateInstance(m_pFaceSdk);
    if (nCode != kFaceResultOk)
    {
        LOG(ERROR) << "Create facesdk instance:" << nCode;
        return false;
    }
    nCode = ::GetFeatureSize(m_pFaceSdk, m_nFeatureize);
    if (nCode != kFaceResultOk)
    {
        LOG(ERROR) << "Get feature size failed:" << nCode;
        return false;
    }
    // create the directory to store the library data
    if (!Basic::File::CreateSingleDirectory(kStorageDir))
    {
        LOG(ERROR) << "Create directory:" << kStorageDir << " to store library data failed:";
        return false;
    }
    load_library();
    return true;
}

void LibraryManagement::Stop()
{
    m_bStop.store(true);
    std::lock_guard<std::mutex> lock(m_mtLib);
    m_mapLibHandle.clear();
    m_poolExcuteTask.Stop();
    DestroyInstance(m_pFaceSdk);
}

int LibraryManagement::AddLibrary(int nLibId, std::string& strResult)
{
    {
        std::lock_guard<std::mutex> lock(m_mtLib);
        std::shared_ptr<LibraryHandle> pHandle(new LibraryHandle(kStorageDir, nLibId));
        m_mapLibHandle[nLibId] = pHandle;
    }
    strResult = ErrorMsgManagement::Instance().GetErrorMsg(CS_OK, strResult);
    return CS_OK;
}

int LibraryManagement::DeleteLibrary(int nLibId)
{
    std::lock_guard<std::mutex> lock(m_mtLib);
    auto itMatch = m_mapLibHandle.find(nLibId);
    if (itMatch == m_mapLibHandle.end())
        return CS_OK;
    itMatch->second->DeleteStorageFile();
    m_mapLibHandle.erase(nLibId);

    return CS_OK;
}

int LibraryManagement::ListLibrary(std::string& strResult)
{
    std::list<int> listLib;
    {
        std::lock_guard<std::mutex> lock(m_mtLib);
        for (const auto& item : m_mapLibHandle)
            listLib.push_back(item.first);
    }
    to_json(listLib, strResult);
    return CS_OK;
}

int LibraryManagement::AddPerson(int nLibId, const std::string& strPicUrl, std::string& strResult)
{
    std::shared_ptr<LibraryHandle> pHandle;
    {
        std::lock_guard<std::mutex> lock(m_mtLib);
        auto itMatch = m_mapLibHandle.find(nLibId);
        if (itMatch == m_mapLibHandle.end())
        {
            strResult = ErrorMsgManagement::Instance().GetErrorMsg(CS_LIBRARY_NOT_EXIST, strResult);
            return CS_OK;
        }
        pHandle = itMatch->second;
    }
    std::shared_ptr<float> pFeature(new float[m_nFeatureize]);
    INVOKE_RETURN(extract_feature(strPicUrl, pFeature), strResult);
    int64_t nPersonId = m_acPerson++;
    pHandle->AddPerson(nPersonId, pFeature);
    JsonHelper::GenerateValue("person_id", nPersonId, strResult);

    return CS_OK;
}

int LibraryManagement::DeletePerson(int nLibId, int64_t nPersonId)
{
    std::lock_guard<std::mutex> lock(m_mtLib);
    auto itMatch = m_mapLibHandle.find(nLibId);
    if (itMatch != m_mapLibHandle.end())
        itMatch->second->DeletePerson(nPersonId);

    return CS_OK;
}

int LibraryManagement::UpdatePerson(int nLibId, 
    int64_t nPersonId, 
    const std::string& strPicUrl, 
    std::string& strResult)
{
    std::shared_ptr<float> pFeature(new float[m_nFeatureize]);
    INVOKE_RETURN(extract_feature(strPicUrl, pFeature), strResult);
    std::lock_guard<std::mutex> lock(m_mtLib);
    auto itMatch = m_mapLibHandle.find(nLibId);
    if (itMatch == m_mapLibHandle.end())
    {
        strResult = ErrorMsgManagement::Instance().GetErrorMsg(CS_LIBRARY_NOT_EXIST, strResult);
        return CS_OK;
    }
    itMatch->second->DeletePerson(nPersonId);
    int64_t nNewPersonId = m_acPerson.fetch_add(1);
    itMatch->second->AddPerson(nNewPersonId, pFeature);
    JsonHelper::GenerateValue("person_id", nNewPersonId, strResult);
    return CS_OK;
}

int LibraryManagement::GetFeatureSize()
{
    return m_nFeatureize;
}

int LibraryManagement::ExtractFeature(const std::string& strPicUrl, std::string& strFeature)
{
    std::shared_ptr<float> pFeature(new float[m_nFeatureize]);
    VERIFY_CODE_RETURN(extract_feature(strPicUrl, pFeature));
    strFeature = serialize_feature(pFeature);
    return CS_OK;
}

int LibraryManagement::Compare1v1(const std::string& strLPicUrl,
    const std::string& strRPicUrl,
    float& nSimilarity)
{
    std::shared_ptr<float> pLFeature(new float[m_nFeatureize]);
    std::shared_ptr<float> pRFeature(new float[m_nFeatureize]);
    VERIFY_CODE_RETURN(extract_feature(strLPicUrl, pLFeature));
    VERIFY_CODE_RETURN(extract_feature(strRPicUrl, pLFeature));

    auto nCode = ::CompareWithFeature(m_pFaceSdk, pLFeature.get(), pRFeature.get(), nSimilarity);
    if (kFaceResultOk != nCode)
    {
        LOG(ERROR) << "CompareWithFeature failed: " << nCode;
        return CS_COMPARE_WITH_FEATURE_FAILED;
    }

    return CS_OK;
}

int LibraryManagement::CompareWithFeature(const std::string& strLFerature,
    const std::string& strRFerature,
    float& nSimilarity)
{
    std::shared_ptr<float> pLFeature(new float[m_nFeatureize]);
    deserialize_feature(strLFerature, pLFeature);
    std::shared_ptr<float> pRFeature(new float[m_nFeatureize]);
    deserialize_feature(strRFerature, pRFeature);
    auto nCode = ::CompareWithFeature(m_pFaceSdk, pLFeature.get(), pRFeature.get(), nSimilarity);
    if (kFaceResultOk != nCode)
    {
        LOG(ERROR) << "CompareWithFeature failed: " << nCode;
        return CS_COMPARE_WITH_FEATURE_FAILED;
    }

    return CS_OK;
}

int LibraryManagement::PushCompareTask(const TaskInfo& task)
{
    {
        std::lock_guard<std::mutex> lock(m_mtTask);
        m_listTask.push_back(task);
    }
    m_cvTask.notify_one();
    return CS_OK;
}

void LibraryManagement::load_library()
{
    std::vector<std::string> vecFile;
    Basic::File::GetFilesOfDir(kStorageDir, vecFile);
    VERIFY_EXPR_RETURN_VOID(!vecFile.empty());
    for (const auto& item : vecFile)
    {
        int nPos = item.find(".data");
        if (std::string::npos == nPos) continue;
        int nLibId = std::stoi(item.substr(0, nPos));
        std::shared_ptr<LibraryHandle> pHandle(new LibraryHandle(kStorageDir, nLibId));
        std::lock_guard<std::mutex> lock(m_mtLib);
        m_mapLibHandle[nLibId] = pHandle;
        // update the id of person
        auto nPeRsonId = pHandle->GetMaxPersonId();
        if (nPeRsonId > m_acPerson.load()) {
            m_acPerson.store(nPeRsonId);
        }
    }
}

int LibraryManagement::extract_feature(const std::string& strPicUrl, std::shared_ptr<float>& pFeature)
{
    VERIFY_EXPR_RETURN((nullptr != m_pFaceSdk), CS_INVALID_INSTANCE);
    std::string strPicData;
    VERIFY_EXPR_RETURN(download_picture(strPicUrl, strPicData),CS_DOWNLOAD_PICTURE_FAILED);
    // byte->mat
    ImageData imgData;
    std::vector<unsigned char> vecData(strPicData.begin(), strPicData.end());
    cv::Mat matFrame = cv::imdecode(vecData, CV_LOAD_IMAGE_COLOR);
    mat_to_image(matFrame, imgData);
    // detect the face of person
    std::vector<FaceInfo> vecFace;
    VERIFY_EXPR_RETURN(detect_face(imgData, vecFace), CS_DETECT_NO_FACE);
    if (vecFace.size() > 1)
        return CS_DETECT_MULTI_FACE;
    if (kFaceResultOk != ::ExtractFeature(m_pFaceSdk, imgData, pFeature.get()))
        return CS_EXTRACT_FEATURE_FAILED;
    return CS_OK;
}

bool LibraryManagement::download_picture(const std::string& strPicUrl, std::string& strPicData)
{
    HttpPara paraHttp;
    paraHttp.strUrl = strPicUrl;
    LibcurlHelper client;
    if (CS_OK != client.Get(paraHttp, strPicData) || strPicData.empty())
        return false;
    return true;
}

bool LibraryManagement::detect_face(const ImageData& imgData, std::vector<FaceInfo>& vecFace)
{
    if (kFaceResultOk == ::Detect(m_pFaceSdk, imgData, vecFace) 
        && !vecFace.empty())
        return true;
    return false;
}

void LibraryManagement::do_task()
{
    while (!m_bStop.load())
    {
        TaskInfo task;
        {
            std::unique_lock<std::mutex> lock(m_mtTask);
            m_cvTask.wait(lock, [=]() {
                return (m_bStop.load() || !m_listTask.empty());
            });
            task = m_listTask.front();
            m_listTask.pop_front();
        }
        if (task.nTaskId > 0)
        {
            if (nullptr == m_pFaceSdk) continue;
            std::vector<char> vecData(task.strPicData.begin(), task.strPicData.end());
            cv::Mat cvMat = cv::imdecode(vecData, CV_LOAD_IMAGE_COLOR);
            ImageData imgData;
            mat_to_image(cvMat, imgData);
            // detect the face of person
            std::vector<FaceInfo> vecFace;
            if(!detect_face(imgData, vecFace)) continue;
            std::vector<std::shared_ptr<float>> vecFeature;
            std::vector<FaceInfo> vecFaceTmp;
            for (const auto& item : vecFace)
            {
                FaceInfo faceInfo;
                faceInfo.width = item.width * 1.5;
                faceInfo.height = item.height * 1.5;
                int nMidX = item.x + item.width / 2;
                int nMidY = item.y + item.height / 2;
                // x
                faceInfo.x = nMidX - faceInfo.width / 2;
                if (faceInfo.x  < 0)faceInfo.x = 0;
                // width
                if (faceInfo.x + faceInfo.width > cvMat.cols)
                    faceInfo.width = cvMat.cols - faceInfo.x;
                // y
                faceInfo.y = nMidY - faceInfo.height / 2;
                if (faceInfo.y < 0)faceInfo.y = 0;
                // height
                if (faceInfo.y + faceInfo.height > cvMat.rows)
                    faceInfo.height = cvMat.rows - faceInfo.y;
                cv::Mat matTmp = cvMat(cv::Rect(faceInfo.x, faceInfo.y, faceInfo.width, faceInfo.height)).clone() ;
                ImageData imgTmp;
                mat_to_image(matTmp, imgTmp);
                std::shared_ptr<float> pFeature(new float[m_nFeatureize]);
                if (kFaceResultOk != ::ExtractFeature(m_pFaceSdk, imgData, pFeature.get())) continue;
                vecFeature.push_back(pFeature);
                vecFaceTmp.push_back(faceInfo);
            }

            std::vector<int> vecLib;
            Basic::SpiltFromString(task.strLibList, vecLib);
            // compare in library
            for (int i = 0; i < vecFaceTmp.size(); ++i)
            {
                std::vector<std::pair<int64_t, float>> vecResult;
                for (const auto& item : vecLib)
                {
                    std::vector<std::pair<int64_t, float>> vecMatch;
                    {
                        std::lock_guard<std::mutex> lock(m_mtLib);
                        auto itMatch = m_mapLibHandle.find(item);
                        if (itMatch == m_mapLibHandle.end())
                            continue;
                        itMatch->second->QueryTopN(vecFeature[i], task.nThreshold, vecMatch, task.nTop);
                    }
                    vecResult.insert(vecResult.end(), vecMatch.begin(), vecMatch.end());
                }
                // sort for result
                std::partial_sort(vecResult.begin(), vecResult.begin() + task.nTop, vecResult.end(), [](
                    const std::pair<int64_t, float> &a, const std::pair<int64_t, float> &b) -> bool
                {
                    return a.second >= b.second;
                });
                // fill the reult of compare task
                Compare::CompareResult rstTask;
                // task info
                Compare::CompareTask infoTask;
                convert_task_info(task, infoTask);
                rstTask.taskInfo = infoTask;
                // face info
                Compare::FaceInfo infoFace;
                convert_face_info(vecFaceTmp[i], infoFace);
                rstTask.infoFace = infoFace;
                if (rstTask.resultMatch.empty() || vecLib.empty())
                {
                    rstTask.resultMatch[0] = 0.0;
                }
                else
                {
                    const size_t nResult = task.nTop > vecResult.size() ? vecResult.size() : task.nTop;
                    for (size_t nIndex = 0; nIndex < nResult; ++nIndex)
                        rstTask.resultMatch[vecResult[nIndex].first] = vecResult[nIndex].second;
                }
                CompareServerImpl::PushCompareResult(rstTask);
            }
        }
    }
}

void LibraryManagement::to_json(const std::list<int>& listData, std::string& strResult)
{
    rapidjson::Document doc(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();
    // info
    {
        rapidjson::Value valueArr(rapidjson::kArrayType);
        for (const auto& item : listData)
        {
            rapidjson::Value valueData(rapidjson::kObjectType);
            valueData.AddMember("lib_id", item, typeAllocate);

            valueArr.PushBack(valueData, typeAllocate);
        }
        doc.AddMember("info", valueArr, typeAllocate);
    }
    doc.AddMember("code", 0, typeAllocate);
    doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
    doc.Accept(writer);
    strResult = buffer.GetString();
}

std::string LibraryManagement::serialize_feature(const std::shared_ptr<float>& pFeature)
{
    std::string strFeature;
    // serize the feature
    {
        std::stringstream ss;
        ss << pFeature;
        strFeature = ss.str();
    }
    return Basic::Base64::Encode((const unsigned char*)&strFeature, strFeature.size());
}

void LibraryManagement::deserialize_feature(const std::string& strFeature, std::shared_ptr<float>& pFeature)
{
    std::stringstream ss;
    ss << Basic::Base64::Decode(strFeature);
    ss.read((char*)pFeature.get(), m_nFeatureize);
}