#include "GlobalConfig.h"
#include <sstream>
#include <easylogging/easylogging++.h>
#include <Basic/CrossPlat.h>
#include "../Common/JsonHelper.h"
#include "../Common/Macro.h"



bool GlobalConfig::Init()
{
    VERIFY_EXPR_RETURN(parse_config(read_file(Config::GetConfigFile())), false);
    return true;
}

std::string GlobalConfig::read_file(const std::string& strFile)
{
    if (0 == CG_access(strFile.c_str(), _A_NORMAL))
    {
        std::fstream fin(strFile, std::ios::in);
        if (fin.is_open())
        {
            fin.seekg(0, std::ios::end);
            int64_t nFileSize = fin.tellp();
            fin.seekg(0, std::ios::beg);
            std::vector<char>vecData(nFileSize + 1, 0);
            fin.read(vecData.data(), nFileSize);
            fin.close();
            LOG(INFO) << "Load config file:" << strFile << " success";
            return vecData.data();
        }
    }
    LOG(ERROR) << "Can't find config file:" << strFile;
    return "";
}

bool GlobalConfig::parse_config(const std::string& datrData)
{
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, datrData, false);
    
    if (doc.HasMember("server_info") && doc["server_info"].IsObject())
    {
        config.LoadFromJson(doc["server_info"]);
    }
    return true;
}

void GlobalConfig::ConfigServer::LoadFromJson(rapidjson::Value& value)
{
    JS_PARSE_OPTION(port, value, Int, port);
}
