//
// Created by kamlesh on 23/10/15.
//

#include <uv.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <fstream>
#include "Settings.h"
#include "Logger.h"

using namespace rapidjson;
using namespace pigeon;

string Settings::ServiceName;
int Settings::WorkerThreads;
string Settings::IPAddress;
int Settings::Port;
string Settings::Filters;
bool  Settings::TcpNoDelay;
bool Settings::UseSsl;
string Settings::SslCertFile;
string Settings::SslKeyFile;
string Settings::DocumentRoot;
string Settings::ResourceLocation;
string Settings::FileUploadLocation;
string Settings::DefaultPage;
string Settings::LogLocation;
string Settings::ApiRoute;
bool Settings::EnableCORS;
unordered_map<string, string> Settings::DbSettings;
unordered_map<string, string> Settings::AppSettings;
unordered_map<string, string> Settings::CORSHeaders;
vector<string> Settings::RequestFilters;
vector<string> Settings::ResponseFilters;
vector<string> Settings::CORSAllowedOrigins;
vector<string> Settings::CORSAllowedMethods;

auto Settings::LoadSettings() -> void {

    try {

        char path[1024];
        size_t sz = 1024;
        uv_cwd(path, &sz);
        string current_path(path);

        current_path.append("/service.json");

        std::ifstream is(current_path.c_str(), std::ios::in | std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(is)), (std::istreambuf_iterator<char>()));

        Document doc;
        doc.Parse(content.c_str());

        ServiceName = doc["ServiceName"].GetString();
        WorkerThreads = doc["WorkerThreads"].GetInt();
        IPAddress = doc["IPAddress"].GetString();
        Port = doc["Port"].GetInt();
		TcpNoDelay = doc["TcpNoDelay"].GetBool();
        UseSsl = doc["UseSsl"].GetBool();
        SslCertFile = doc["SslCertFile"].GetString();
        SslKeyFile = doc["SslKeyFile"].GetString();
        DocumentRoot = doc["DocumentRoot"].GetString();
        ResourceLocation = doc["ResourceLocation"].GetString();
        FileUploadLocation = doc["FileUploadLocation"].GetString();
        DefaultPage = doc["DefaultPage"].GetString();
        ApiRoute = doc["ApiRoute"].GetString();
        LogLocation = doc["LogLocation"].GetString();
        EnableCORS = doc["EnableCORS"].GetBool();



        for (Value::ConstMemberIterator it = doc["DbSettings"].MemberBegin();
             it != doc["DbSettings"].MemberEnd(); ++it) {
            DbSettings.emplace(std::pair<string, string>(it->name.GetString(), it->value.GetString()));
        }

        for (Value::ConstMemberIterator it = doc["CORSHeaders"].MemberBegin();
             it != doc["CORSHeaders"].MemberEnd(); ++it) {
                CORSHeaders.emplace(std::pair<string, string>(it->name.GetString(), it->value.GetString()));
        }

        for (Value::ConstMemberIterator it = doc["AppSettings"].MemberBegin();
             it != doc["AppSettings"].MemberEnd(); ++it) {
            AppSettings.emplace(std::pair<string, string>(it->name.GetString(), it->value.GetString()));
        }


        const Value& req_filters = doc["RequestFilters"];
        assert(req_filters.IsArray());
        for (SizeType i = 0; i < req_filters.Size(); i++) {
            RequestFilters.push_back(req_filters[i].GetString());
        }

        const Value& res_filters = doc["ResponseFilters"];
        assert(res_filters.IsArray());
        for (SizeType i = 0; i < res_filters.Size(); i++) {
            ResponseFilters.push_back(res_filters[i].GetString());
        }


    }
    catch (std::exception &ex) {
        Logger::Get()->Write(LogType::Error, ex.what());
    }


}



