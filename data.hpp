#include <string>
#include <ctime>
#include <fstream>
#include <direct.h>

namespace DATA {
    std::string PATH = "data/{date}.csv";

    bool isFileExist(const std::string& path) {
        std::ifstream file(path);
        return file.good(); // check if file can be opened
    }

    std::string getPath() {
        time_t now = time(0);
        tm* ltm = localtime(&now);

        char date[20];
        sprintf(date, "%04d-%02d-%02d", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);

        std::string path = PATH;
        size_t pos = path.find("{date}");
        if (pos != std::string::npos) {
            path.replace(pos, 6, date);
        }
        return path;
    }


    void ensureDataDir() {
        FILE* f = fopen("data", "r");
        if (!f) _mkdir("data");
        else fclose(f);
    }

    void write(std::string user, std::string category, int spend, std::string note) {
        ensureDataDir();
        std::string path = getPath();
        if (!isFileExist(path)) {
            FILE* f = fopen(path.c_str(), "w");
            if (f) {
                fprintf(f, "user,category,spend,note\n");
                fclose(f);
            }
        }

        FILE* f = fopen(path.c_str(), "a");
        if (f) {
            fprintf(f, "%s,%s,%d,%s\n", user.c_str(), category.c_str(), spend, note.c_str());
            fclose(f);
        }
    }

    std::string get_by_date(std::string date, std::string& out) {
        std::string path = PATH;
        size_t pos = path.find("{date}");
        if (pos != std::string::npos) {
            path.replace(pos, 6, date);
        }

        FILE* f = fopen(path.c_str(), "r");
        if (f) {
            char line[1024];
            while (fgets(line, sizeof(line), f)) {
                out += line;
            }
            fclose(f);
        }
        return out;
    }

    std::string get_by_user(std::string user, std::string& out) {
        std::string path = getPath();
        FILE* f = fopen(path.c_str(), "r");
        if (f) {
            char line[1024];
            while (fgets(line, sizeof(line), f)) {
                std::string str_line(line);
                if (str_line.find(user + ",") == 0) {
                    out += line;
                }
            }
            fclose(f);
        }
        return out;
    }

}