
#include <string>
#include <cstring>

// WARNING: 栈溢出勿设置过大 - 踩坑
struct HEADER {
    char statusCode[64]="200 OK";
    char contentType[64]="text/html";
    int contentLength=0;
    bool keepAlive=false;
    char connection[64]="close";
    char PATH[1024];
    char QUERY[1024]; // api/... ? query data
    char Cookie[1024];
    char Auth[1024];
    char method[8];
    char Data[4096];
};

namespace Parser {

    HEADER parseRequest (const char* request) {
        HEADER header;
        std::string req(request);

        // split headers and body
        std::string body;
        size_t bodyPos = req.find("\r\n\r\n");
        if (bodyPos != std::string::npos) {
            body = req.substr(bodyPos + 4);
            req.erase(bodyPos);
        }
        header.contentLength = body.length();

        // parse request line
        size_t eol = req.find("\r\n");
        if (eol != std::string::npos) {
            std::string firstLine = req.substr(0, eol);
            sscanf(firstLine.c_str(), "%7s %1023s", header.method, header.PATH);
            req.erase(0, eol + 2);
        }

        // 简易解析HTTPHeader
        while (!req.empty()) {
            size_t pos = req.find("\r\n");
            if (pos == std::string::npos) break;
            std::string line = req.substr(0, pos);
            req.erase(0, pos + 2);
            if (line.empty()) break;

            if (line.find("Cookie:") == 0)
                sscanf(line.c_str(), "Cookie: %1023s", header.Cookie);
            else if (line.find("Authorization:") == 0)
                sscanf(line.c_str(), "Authorization: %1023s", header.Auth);
            else if (line.find("Connection:") == 0)
                sscanf(line.c_str(), "Connection: %63s", header.connection);
        }

        // GET请求Query strng
        char* qmark = strchr(header.PATH, '?');
        if (qmark) {
            *qmark = '\0';
            strncpy(header.QUERY, qmark + 1, sizeof(header.QUERY) - 1);
        } else {
            header.QUERY[0] = '\0';
        }

        // POST请求Body
        if (strcmp(header.method, "POST") == 0) {
            strncpy(header.Data, body.c_str(), sizeof(header.Data) - 1);
        }

        return header;
    }

    void debug_outputHeader (HEADER header) {
        printf("-----------------------------------------\n");
        printf("|  Parsed header:\n");
        printf("|     PATH: %s\n", header.PATH);
        printf("|     Cookie: %s\n", header.Cookie);
        printf("|     Auth: %s\n", header.Auth);
        printf("|     Connection: %s\n", header.connection);
        printf("|     Data: %s\n", header.Data);
        printf("-----------------------------------------\n");
    }

}