#include "person.h"
#include <soci/soci.h>
#include <easylogging/easylogging++.h>
#include <JsonHelper.h>
#include <Macro.h>
#include <SociHelp.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Config/GlobalConfig.h"
#include "../Cache/session/Session.h"
#include "../Client/CompareClient.h"
#include "../Cache/ConnectionPool/DatabaseConnPool.h"
#include "Hanz2Piny/Hanz2Piny.h"


namespace soci
{
    template<>
    struct type_conversion<Service::PersonInfo>
    {
        typedef values base_type;
        static void from_base(const values& v, indicator ind, Service::PersonInfo& person)
        {
            person.nPersonId = v.get<int64_t>("id");
            person.strName = v.get<std::string>("name");
            person.strPiny = v.get<std::string>("pinyin");
            person.nLibId = v.get<int>("lib_id");
            person.strLibName = v.get<std::string>("lib_name");
            person.nSex = v.get<int>("sex");
            person.strCardNo = v.get<std::string>("card_no");
            person.strPhone = v.get<std::string>("phone");
            person.nCategory = v.get<int>("category");
            person.nPicId = v.get<int64_t>("pic_id");
            person.strPicUrl = v.get<std::string>("pic_url");
            if (!person.strPicUrl.empty())
                person.strPicUrl.insert(0, CFG_STORAGE_SERVER.strUrlPrefix);
            person.strRemark = v.get<std::string>("remark");
            person.strCreateTime = SociHelper::FetchTime(v, "create_time");
        }
    };
}

namespace JsonHelper
{
    static void to_json(const Service::PersonInfo& infoPerson,
        rapidjson::Value& value,
        rapidjson::Document::AllocatorType &allocator)
    {
        value.AddMember("lib_id", infoPerson.nLibId, allocator);
        value.AddMember("lib_name", rapidjson::Value(infoPerson.strLibName.c_str(), allocator), allocator);
        value.AddMember("person_id", infoPerson.nPersonId, allocator);
        value.AddMember("name", rapidjson::Value(infoPerson.strName.c_str(), allocator), allocator);
        value.AddMember("pinyin", rapidjson::Value(infoPerson.strPiny.c_str(), allocator), allocator);
        value.AddMember("sex", infoPerson.nSex, allocator);
        value.AddMember("card_no", rapidjson::Value(infoPerson.strCardNo.c_str(), allocator), allocator);
        value.AddMember("phone", rapidjson::Value(infoPerson.strPhone.c_str(), allocator), allocator);
        value.AddMember("category", infoPerson.nCategory, allocator);
        value.AddMember("pic_id", infoPerson.nPicId, allocator);
        value.AddMember("pic_url", rapidjson::Value(infoPerson.strPicUrl.c_str(), allocator), allocator);
        value.AddMember("remark", rapidjson::Value(infoPerson.strRemark.c_str(), allocator), allocator);
        value.AddMember("create_time", rapidjson::Value(infoPerson.strCreateTime.c_str(), allocator), allocator);
    }

}


using namespace Service;
int PersonManagement::AddPerson(const std::string& strMsg, std::string& strRst)
{
    PersonInfo infoPerson;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoPerson.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoPerson.nLibId, doc, Int, lib_id, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoPerson.strName, doc, String, name, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(infoPerson.nSex, doc, Int, sex);
    JS_PARSE_REQUIRED_RETURN(infoPerson.strPicUrl, doc, String, pic_url, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(infoPerson.strCardNo, doc, String, card_no);
    JS_PARSE_OPTION(infoPerson.strPhone, doc, String, phone);
    JS_PARSE_OPTION(infoPerson.nCategory, doc, Int, category);
    JS_PARSE_OPTION(infoPerson.strRemark, doc, String, remark);

    VERIFY_CODE_RETURN(check_person_exist(infoPerson.nLibId, infoPerson.strCardNo));
    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(infoPerson.strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(add_person(infoPerson.nLibId, infoPerson.strPicUrl, infoPerson.nPicId));

    infoPerson.strPiny = Hanz2Piny::GetPinyByUtf8(infoPerson.strName);
    VERIFY_CODE_RETURN(add_person(infoPerson));
    return CG_OK;
}

int PersonManagement::DeletePerson(const std::string& strMsg, std::string& strRst)
{
    std::string strSession;
    int nLibId = 0;
    int64_t nPersonId = 0;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(nLibId, doc, Int, lib_id, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(nPersonId, doc, Int64, person_id, strRst, CG_INVALID_PARA);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    Client::CompareClient::Instance().DeletePerson(nLibId, nPersonId);
    VERIFY_CODE_RETURN(delete_person(nLibId, nPersonId));

    return CG_OK;
}

int PersonManagement::UpdatePerson(const std::string& strMsg, std::string& strRst)
{
    PersonInfo infoPerson;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoPerson.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoPerson.nLibId, doc, Int, lib_id, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoPerson.nPersonId, doc, Int64, lib_id, strRst, CG_INVALID_PARA);

    std::stringstream ss;
    ss << "update ft_person set visible = 1";
    JS_PARSE_OPTION_UPDATE_STRING(infoPerson.strName, doc, String, name, ss);
    JS_PARSE_OPTION_UPDATE_INT(infoPerson.nSex, doc, Int, sex, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoPerson.strCardNo, doc, String, card_no, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoPerson.strPhone, doc, String, phone, ss);
    JS_PARSE_OPTION_UPDATE_INT(infoPerson.nCategory, doc, Int, category, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoPerson.strRemark, doc, String, remark, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoPerson.strPicUrl, doc, String, pic_url, ss);

    if (!infoPerson.strName.empty())
    {
        infoPerson.strPiny = Hanz2Piny::GetPinyByUtf8(infoPerson.strName);
        ss << ",`pinyin` = " << infoPerson.strPiny;
    }
    VERIFY_CODE_RETURN(update_person(infoPerson, ss.str()));
    return CG_OK;
}

int PersonManagement::QueryPersonList(const std::string& strMsg, std::string& strRst)
{
    PersonQueryCond condQuery;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(condQuery.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(condQuery.nPage, doc, Int, page_no, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(condQuery.nPageSize, doc, Int, page_size, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(condQuery.strName, doc, String, username);
    JS_PARSE_OPTION(condQuery.strCardNo, doc, String, card_no);
    JS_PARSE_OPTION(condQuery.nCategory, doc, Int, category);
    JS_PARSE_OPTION(condQuery.strPhone, doc, String, phone);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(condQuery.strSession), CG_INVALID_SESSION);
    int64_t nTotalCnt = 0;
    PersonList listPerson;
    VERIFY_CODE_RETURN(query_person_list(condQuery, listPerson, nTotalCnt));

    JsonHelper::GenerateList(condQuery.nPage, condQuery.nPageSize, nTotalCnt, listPerson, strRst);
    return CG_OK;
}

int PersonManagement::check_person_exist(
    int nLibId, 
    const std::string& strCardNo, 
    int64_t nPersonId)
{
    if (strCardNo.empty()) return CG_OK;
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "select id from ft_person where visible=1"
            << " and lib_id=:lib_id and card_no=:card_no"
            <<" and id<>:id"
            , soci::use(nLibId)
            , soci::use(strCardNo)
            , soci::use(nPersonId);
        if (conn.got_data()) return CG_PERSON_CARD_NO_EXIST;
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Check person's card-no exception:" << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int PersonManagement::add_person(int nLibId, const std::string& strPicUrl, int64_t& nPicId)
{
    std::string strTmpUrl = strPicUrl;
    if (strTmpUrl.empty()) return CG_INVALID_PERSON_PHOTO;
    strTmpUrl.insert(0, CFG_STORAGE_SERVER.strUrlPrefix);
    std::string strResult;
    VERIFY_RETURN_OPTION(Client::CompareClient::Instance().AddPerson(nLibId, strTmpUrl, strResult), CG_ADD_LIBRARY_COMPARE_FAILED);

    rapidjson::Document docRst;
    JS_PARSE_OBJECT_RETURN(docRst, strResult, CG_ADD_PERSON_FAILED);
    int code = CG_OK;
    JS_PARSE_OPTION_RETURN(code, docRst, Int, code, CG_ADD_PERSON_FAILED);
    if (CG_OK != code)
    {
        std::string strMsg;
        JS_PARSE_OPTION_RETURN(strMsg, docRst, String, msg, CG_INVALID_PARA);
        LOG(ERROR) << "Add person failed,error:" << strMsg;
        return CG_ADD_PERSON_FAILED;
    }
    VERIFY_EXPR_RETURN(CG_OK == code, CG_ADD_PERSON_FAILED);
    JS_CHECK_FIELD_RETURN(docRst, info, Object, CG_ADD_PERSON_FAILED);
    JS_CHECK_FIELD_RETURN(docRst["info"], person_id, Int64, CG_ADD_PERSON_FAILED);
    JS_PARSE_OPTION_RETURN(nPicId, docRst["info"], Int64, person_id, CG_ADD_PERSON_FAILED);

    return CG_OK;
}

int PersonManagement::add_person(const PersonInfo& infoPerson)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        DB_BEGIN(conn);
        conn << "insert into ft_person(lib_id,name,sex,card_no,phone,category"
            ",pinyin,remark)"
            << " values(:lib_id,:name,:sex,:card_no,:phone,:category"
            ",:pinyin,:remark)"
            , soci::use(infoPerson.nLibId)
            , soci::use(infoPerson.strName)
            , soci::use(infoPerson.strPiny)
            , soci::use(infoPerson.nSex)
            , soci::use(infoPerson.strCardNo)
            , soci::use(infoPerson.strPhone)
            , soci::use(infoPerson.nCategory)
            , soci::use(infoPerson.strRemark);
        int64_t nPersonId;
        conn << "select last_insert_id()", soci::into(nPersonId);
        conn << "insert into ft_picture(pic_id,person_id,pic_url)"
            << " values(:pic_id,:person_id,:pic_url)"
            , soci::use(infoPerson.nPicId)
            , soci::use(nPersonId)
            , soci::use(infoPerson.strPicUrl);
        conn << "update ft_library set person_count = person_count + 1"
            << " where visible = 1 and id=:id"
            , soci::use(infoPerson.nLibId);
        DB_COMMIT(conn);
    }
    catch (const std::exception& e)
    {
        DB_ROLLBACK(conn);
        LOG(ERROR) << "Add person exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int PersonManagement::delete_person(int nLibId, int64_t nPersonId)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        DB_BEGIN(conn);
        conn << "update ft_person set visible = 0 where visible = 1"
            << " and id=:id"
            , soci::use(nPersonId);
        conn << "update ft_picture set visible = 0 where visible = 1"
            << " and person_id=:person_id"
            , soci::use(nPersonId);
        conn << "update ft_library set person_count = person_count - 1"
            << " where visible = 1 and id=:id"
            , soci::use(nLibId);
        DB_COMMIT(conn);
    }
    catch (const std::exception& e)
    {
        DB_ROLLBACK(conn);
        LOG(ERROR) << "Delete person:" << nPersonId << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int PersonManagement::update_person(const PersonInfo& infoPerson, const std::string& strUpdateSql)
{
    VERIFY_CODE_RETURN(check_person_exist(infoPerson.nLibId, infoPerson.strCardNo));
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        int64_t nPicId = 0;
        int64_t nPicIdOrign = 0;
        if (!infoPerson.strPicUrl.empty())
        {
            std::string strPicUrl;
            conn << "select pic_id,pic_url from ft_picture where visible = 1"
                << " and person_id =: person_id"
                , soci::into(nPicIdOrign)
                , soci::into(strPicUrl)
                , soci::use(infoPerson.nPersonId);
            if (conn.got_data())
            {
                if (strPicUrl != infoPerson.strPicUrl)
                {
                    Client::CompareClient::Instance().DeletePerson(infoPerson.nLibId, infoPerson.nPersonId);
                    VERIFY_CODE_RETURN(add_person(infoPerson.nLibId, infoPerson.strPicUrl, nPicId));
                }
            }
        }
        DB_BEGIN(conn);
        conn << strUpdateSql
            << " where person_id = :person_id"
            , soci::use(infoPerson.nPersonId);
        if (nPicId > 0)
        {
            conn << "update ft_picture set visible = 0"
                << " where pic_id=:pic_id"
                , soci::use(nPicIdOrign);
            conn << "insert into ft_picture(pic_id,person_id,pic_url)"
                << " values(:pic_id,:person_id,:pic_url)"
                , soci::use(nPicId)
                , soci::use(infoPerson.nPersonId)
                , soci::use(infoPerson.strPicUrl);
        }
        DB_COMMIT(conn);
    }
    catch (const std::exception& e)
    {
        DB_ROLLBACK(conn);
        LOG(ERROR) << "Update person:"<< infoPerson.nPersonId <<" exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int PersonManagement::query_person_list(const PersonQueryCond& condQuery,
    PersonList& listPerson, 
    int64_t& nCnt)
{
    std::string strSqlCond;
    {
        std::stringstream ss;
        ss << " where t_person.visible = 1";
        if (!condQuery.strName.empty())
            ss << " and t_person.name like '%" << condQuery.strName << "%'";
        if (!condQuery.strCardNo.empty())
            ss << " and card_no = '" << condQuery.strCardNo << "'";
        if (condQuery.nCategory > 0)
            ss << " and category = " << condQuery.nCategory;
        if (!condQuery.strPhone.empty())
            ss << " and phone like '%" << condQuery.strPhone << "%'";
        strSqlCond = ss.str();
    }
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "select count(*) from ft_person t_person"
            << strSqlCond
            , soci::into(nCnt);
        if (0 == nCnt) return CG_OK;
        PersonInfo infoPerson;
        soci::statement st = (conn.prepare
            << "select t_person.*,t_lib.name as lib_name,pic_id,pic_url from ft_person t_person"
            << " left join ft_library t_lib on t_lib.lib_id = t_person.lib_id"
            << " left join ft_picture t_pic on t_pic.person_id = t_person.id"
            << strSqlCond
            << " limit " << (condQuery.nPage - 1) * condQuery.nPageSize
            << " ," << condQuery.nPageSize
            , soci::into(infoPerson));
        st.execute();
        while (st.fetch())
            listPerson.push_back(infoPerson);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Query person list exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}
