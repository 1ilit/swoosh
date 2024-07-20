#include "HTTPClient.h"
#include <vector>
#include <sstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

HTTPClient::HTTPClient() : m_ctx(NULL), m_ssl(NULL), m_socket(-1)
{
    init();
}

HTTPClient::~HTTPClient()
{
    cleanup();
}

void HTTPClient::get(std::string hostname, std::string path)
{
    bool created_socket = create_socket(hostname);

    if (!created_socket)
    {
        std::cout << "Womp womp\n";
        return;
    }

    bool did_handshake = handshake();
    if (!did_handshake)
    {
        std::cout << "Womp womp 2\n";
        return;
    }

    std::stringstream ss;
    ss << "GET " << path << " HTTP/1.1\r\n"
       << "Host: " << hostname << "\r\n"
       << "Accept: application/json\r\n"
       << "Connection: close\r\n"
       << "\r\n";

    bool sent_request = send_request(ss.str());

    if (!sent_request)
    {
        std::cout << "Womp womp 3\n";
        return;
    }

    std::string response = read_response();

    cleanup();

    std::cout << response;
}

bool HTTPClient::send_request(std::string request)
{
    if (SSL_write(m_ssl, request.c_str(), request.length()) <= 0)
    {
        std::cout << "Error sending request.\n";
        return false;
    }

    return true;
}

std::string HTTPClient::read_response()
{
    const int buffer_size = 4096;
    std::vector<char> buffer(buffer_size);
    std::string response;
    int bytes_read;

    while ((bytes_read = SSL_read(m_ssl, buffer.data(), buffer_size - 1)) > 0)
    {
        response.append(buffer.data(), bytes_read);
    }

    if (bytes_read < 0)
    {
        std::cout << "Error reading response.\n";
        ERR_print_errors_fp(stderr);
    }

    return response;
}

void HTTPClient::init()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    m_ctx = SSL_CTX_new(TLS_client_method());
    if (!m_ctx)
    {
        std::cout << "Error creating SSL context." << std::endl;
    }
}

void HTTPClient::cleanup()
{
    if (m_ssl)
    {
        SSL_shutdown(m_ssl);
        SSL_free(m_ssl);
        m_ssl = nullptr;
    }
    if (m_socket != -1)
    {
        close(m_socket);
        m_socket = -1;
    }
}

bool HTTPClient::create_socket(std::string hostname)
{
    struct hostent *host = gethostbyname(hostname.c_str());
    if ((host == NULL) || (host->h_addr == NULL))
    {
        std::cout << "Error retrieving DNS information.\n";
        return false;
    }

    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(m_port);
    memcpy(&client.sin_addr, host->h_addr, host->h_length);

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0)
    {
        std::cout << "Error creating socket.\n";
        return false;
    }

    if (connect(m_socket, (struct sockaddr *)&client, sizeof(client)) < 0)
    {
        close(m_socket);
        std::cout << "Could not connect.\n";
        return false;
    }

    return true;
}

bool HTTPClient::handshake()
{
    m_ssl = SSL_new(m_ctx);
    SSL_set_fd(m_ssl, m_socket);

    if (SSL_connect(m_ssl) <= 0)
    {
        std::cout << "Error establishing SSL connection.\n";
        ERR_print_errors_fp(stderr);
        return false;
    }

    return true;
}
