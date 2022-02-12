/**************************************************************
* @brief:       functions of JSON
* @date:         20200124
* @update:
* @auth:         Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <rapidjson/document.h>
#include <rapidjson/writer.h>



namespace JsonHelper
{
    // generate list value
    template<class TList>
    static void GenerateList(int nPage, int nPageSize, int64_t nCnt, const TList& listData, std::string& strRst)
    {
        rapidjson::Document doc(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();
        doc.AddMember("code", 0, typeAllocate);
        // info
        {
            rapidjson::Value valueInfo(rapidjson::kObjectType);
            rapidjson::Value valueArr(rapidjson::kArrayType);
            for (const auto& item : listData)
            {
                rapidjson::Value valueData(rapidjson::kObjectType);
                to_json(item, valueData, typeAllocate);
                valueArr.PushBack(valueData, typeAllocate);
            }
            valueInfo.AddMember("page_no", nPage, typeAllocate);
            valueInfo.AddMember("page_size", nPageSize, typeAllocate);
            valueInfo.AddMember("total_count", nCnt, typeAllocate);
            valueInfo.AddMember("items", valueArr, typeAllocate);

            doc.AddMember("info", valueInfo, typeAllocate);
        }
        doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
        doc.Accept(writer);
        strRst = buffer.GetString();
    }

    // generate list value
    template<class TList>
    static void GenerateListResult(int64_t nCnt, const TList& listData, std::string& strRst)
    {
        rapidjson::Document doc(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();
        doc.AddMember("code", 0, typeAllocate);
        // info
        {
            rapidjson::Value valueInfo(rapidjson::kObjectType);
            rapidjson::Value valueArr(rapidjson::kArrayType);
            for (const auto& item : listData)
            {
                rapidjson::Value valueData(rapidjson::kObjectType);
                to_json(item, valueData, typeAllocate);
                valueArr.PushBack(valueData, typeAllocate);
            }
            valueInfo.AddMember("total_count", nCnt, typeAllocate);
            valueInfo.AddMember("items", valueArr, typeAllocate);

            doc.AddMember("info", valueInfo, typeAllocate);
        }
        doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
        doc.Accept(writer);
        strRst = buffer.GetString();
    }

    // generate int value
    template<class T>
    static void GenerateValue(const std::string& strKey, T nValue, std::string& strRst)
    {
        rapidjson::Document doc(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();
        doc.AddMember("code", 0, typeAllocate);
        // info
        {
            rapidjson::Value valueInfo(rapidjson::kObjectType);
            valueInfo.AddMember(rapidjson::Value(strKey.c_str(), typeAllocate), nValue, typeAllocate);

            doc.AddMember("info", valueInfo, typeAllocate);
        }
        doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
        doc.Accept(writer);
        strRst = buffer.GetString();
    }

    // generate string value
    static void GenerateStringValue(const std::string& strKey, const std::string& strValue, std::string& strRst)
    {
        rapidjson::Document doc(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();
        doc.AddMember("code", 0, typeAllocate);
        // info
        {
            rapidjson::Value valueInfo(rapidjson::kObjectType);
            valueInfo.AddMember(rapidjson::Value(strKey.c_str(), typeAllocate), rapidjson::Value(strValue.c_str(), typeAllocate), typeAllocate);

            doc.AddMember("info", valueInfo, typeAllocate);
        }
        doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
        doc.Accept(writer);
        strRst = buffer.GetString();
    }

    // generate object value
    template<class TObject>
    static void GenerateObject(const TObject& dataObject, std::string& strRst)
    {
        rapidjson::Document doc(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();
        doc.AddMember("code", 0, typeAllocate);
        // info
        {
            rapidjson::Value valueInfo(rapidjson::kObjectType);
            to_json(dataObject, valueInfo, typeAllocate);

            doc.AddMember("info", valueInfo, typeAllocate);
        }
        doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
        doc.Accept(writer);
        strRst = buffer.GetString();
    }

}