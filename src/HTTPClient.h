#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

class HTTPClient
{
public:
    HTTPClient();
    ~HTTPClient();
    void get(std::string hostname, std::string path);

private:
    void init();
    void cleanup();
    bool create_socket(std::string hostname);
    bool handshake();
    bool send_request(std::string request);
    std::string read_response();

    SSL_CTX *m_ctx;
    SSL *m_ssl;
    int m_socket;
    const int m_port = 443;
};