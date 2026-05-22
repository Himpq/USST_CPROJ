#include <stdio.h>
#include <string.h>
#include <string>
#include "data.hpp"


namespace API {

    std::string get_param(std::string& query, std::string key) {
        size_t pos = query.find(key + "="); // ?xxx=123&bb=345 find "xxx="

        if (pos == std::string::npos) 
            return "";

        size_t start = pos + key.length() + 1; // ?[x]xx=123&bb=345
        size_t end = query.find('&', start);   // ?xxx=123[&]bb=345
        if (end == std::string::npos) end = query.length(); // [START]xxx=123[END]

        std::string _new = query.substr(start, end - start); // return "123"
        return _new;
    }

    const char* _404_res = "<h1>404 Not Found</h1>";

    void debug_test (HEADER& rqHeader, HEADER& rdHeader, char* buff) {
        printf("API called with PATH: %s\n", rqHeader.PATH);
        sprintf(buff, "Hello from API! You requested %s", rqHeader.PATH);

        char* resp = "<h1>Hello</h1>";
        strcpy(buff, resp);
    }

    void write (HEADER& rqHeader, HEADER& rdHeader, char* buff) {
        std::string QUERY = rqHeader.QUERY;
        // /api/write?user=xxx&category=餐饮&spend=100&note=yyy

        try {

            std::string user, category, note;
            int spend = 0;

            user = get_param(QUERY, "user");
            category = get_param(QUERY, "category");
            note = get_param(QUERY, "note");
            std::string spend_str = get_param(QUERY, "spend");
            spend = std::stoi(spend_str);

            if (user.empty() || category.empty()) {
                strcpy(buff, "Parameter is empty");
                return;
            }

            DATA::write(user, category, spend, note);

            strcpy(buff, "Saved");

        } catch (...) {
            strcpy(buff, "Argument error");
        }
    }

    void get_by_date (HEADER& rqHeader, HEADER& rdHeader, char* buff) {
        std::string QUERY = rqHeader.QUERY;
        // /api/get_by_date?date=2024-06-01

        std::string date = get_param(QUERY, "date");
        if (date.empty()) {
            strcpy(buff, "Parameter is empty");
            return;
        }

        std::string out;
        DATA::get_by_date(date, out);
        if (out.empty()) {
            strcpy(buff, "No data for this date");
            return;
        }

        strcpy(buff, out.c_str());
    }

    void get_by_user (HEADER& rqHeader, HEADER& rdHeader, char* buff) {
        std::string QUERY = rqHeader.QUERY;
        // /api/get_by_user?user=xxx

        std::string user = get_param(QUERY, "user");
        if (user.empty()) {
            strcpy(buff, "Parameter is empty");
            return;
        }

        std::string out;
        DATA::get_by_user(user, out);
        if (out.empty()) {
            strcpy(buff, "No data for this user");
            return;
        }

        strcpy(buff, out.c_str());
    }

    void return_file_content (HEADER& rqHeader, HEADER& rdHeader, char* buff) {
        std::string path = "frontend/static/myjs.js";
        FILE* f = fopen(path.c_str(), "rb");
        if (f) {
            int n = fread(buff, 1, 16383, f);
            fclose(f);
            buff[n] = '\0';
        } else {
            strcpy(buff, _404_res);
        }
    }

    struct Route {
        const char* method;
        const char* path;
        void (*handler)(HEADER&, HEADER&, char*);
    };

    Route routes[] = {
        {"GET", "/api/test", debug_test},
        {"GET", "/api/write", write},
        {"GET", "/api/get_by_date", get_by_date},
        {"GET", "/api/get_by_user", get_by_user},
        {"GET", "/static/myjs.js", return_file_content},
    };

    void deliver (HEADER& rqHeader, HEADER& rdHeader, char* buff) {
        // ROUTE
        std::string PATH(rqHeader.PATH);

        if (PATH == "/") {
            FILE* f = fopen("frontend/index.html", "rb");
            if (f) {
                int n = fread(buff, 1, 16383, f);
                fclose(f);
                buff[n] = '\0';
            }
        } else {

            for (const auto& r : routes) {
                if (strcmp(rqHeader.method, r.method) == 0 && PATH == r.path) {
                    r.handler(rqHeader, rdHeader, buff);
                    return;
                }
            }

            strcpy(buff, _404_res); // 404
        }
    }

    

}
