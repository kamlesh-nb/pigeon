//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_APP_SETTINGS_H
#define PIGEON_APP_SETTINGS_H

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;


namespace pigeon {

    class Settings {

    public:

        static string ServiceName;
        static int WorkerThreads;
        static string IPAddress;
        static int Port;
        static string Filters;
		static bool TcpNoDelay;
        static bool UseSsl;
        static string SslCertFile;
        static string SslKeyFile;
        static string DocumentRoot;
        static string ResourceLocation;
        static string FileUploadLocation;
        static string DefaultPage;
        static string LogLocation;
        static string ApiRoute;
        static bool EnableCORS;
        static unordered_map<string, string> DbSettings;
        static unordered_map<string, string> AppSettings;
        static unordered_map<string, string> CORSHeaders;
        static vector<string> RequestFilters;
        static vector<string> ResponseFilters;
        static vector<string> CORSAllowedOrigins;
        static vector<string> CORSAllowedMethods;
        static auto LoadSettings() -> void;


    };

}


#endif //PIGEON_APP_SETTINGS_H
