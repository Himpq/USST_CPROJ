

#include <iostream>
/* Rely on Winsock2 */
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#include "parser.hpp"
#include "api.hpp"

const char* resp = "HTTP/1.1 200 OK\r\nContent-Length: ";
const char* addr = "127.0.0.1";
const int port = 1180;


char* buildHeader(HEADER header) {
    char* buf = new char[1024];
    sprintf(buf, "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: %s\r\n\r\n",
        header.statusCode, header.contentType, header.contentLength, header.keepAlive ? "keep-alive" : "close");
    return buf;
}

void handle (SOCKET s) {
    char buf[4096] = {0};
    recv(s, buf, sizeof(buf) - 1, 0);

    // 解析HTTP Header
    HEADER reqHeader = Parser::parseRequest(buf);
    Parser::debug_outputHeader(reqHeader);
    printf("Body: %s\n\n", reqHeader.Data);

    char responseContent[16384] = {0};
    HEADER respondHeader;

    // Route转发
    API::deliver(reqHeader, respondHeader, responseContent);
    
    // 返回信息
    respondHeader.contentLength = strlen(responseContent);

    char* responseHeader = buildHeader(respondHeader);

    send(s, responseHeader, strlen(responseHeader), 0);
    send(s, responseContent, respondHeader.contentLength, 0);

    delete[] responseHeader;
}

int main () {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

    // 
    struct sockaddr_in sAddr;
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(port);
    sAddr.sin_addr.s_addr = inet_addr(addr);

    // bind and listen
    bind(s, (struct sockaddr*)&sAddr, sizeof(sAddr));
    listen(s, 5);

    printf("Server started at %s:%d\n", addr, port);

    while (1) {
            SOCKET c = accept(s, NULL, NULL);
            if (c == INVALID_SOCKET) {
                std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
                continue;
            }
            std::cout << "Client connected!" << std::endl;
            handle(c);
            closesocket(c);
    }
    WSACleanup();
}
