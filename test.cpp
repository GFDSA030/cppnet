#include <iostream>
#include <unet.h>
#include <http.h>
#include <unistd.h>
#include <chrono>
#include <thread>

namespace console
{
    constexpr char reset[] = "\033[0m\033[39m\033[49m";
    namespace decoration
    {
        constexpr char bold[] = "\033[1m";
        constexpr char underline[] = "\033[4m";
        constexpr char blink[] = "\033[5m";
        constexpr char reversed[] = "\033[7m";
        constexpr char reset[] = "\033[0m";
    }
    namespace colors
    {
        constexpr char black[] = "\033[30m";
        constexpr char red[] = "\033[31m";
        constexpr char green[] = "\033[32m";
        constexpr char yellow[] = "\033[33m";
        constexpr char blue[] = "\033[34m";
        constexpr char masenda[] = "\033[35m";
        constexpr char sian[] = "\033[36m";
        constexpr char white[] = "\033[37m";
        constexpr char reset[] = "\033[39m";
    }
    namespace bg_colors
    {
        constexpr char black[] = "\033[40m";
        constexpr char red[] = "\033[41m";
        constexpr char green[] = "\033[42m";
        constexpr char yellow[] = "\033[43m";
        constexpr char blue[] = "\033[44m";
        constexpr char masenda[] = "\033[45m";
        constexpr char sian[] = "\033[46m";
        constexpr char gray[] = "\033[47m";
        constexpr char reset[] = "\033[49m";
    }
}

void fnc(unet::net_core &nc, void *udata)
{
    // std::string request = nc.recv_all();
    char buffer[4096];
    int bytes_received = nc.recv_data(buffer, sizeof(buffer) - 1);
    std::cout << console::colors::red << "from " << unet::ip2str(nc.remote())
              << "\nReceived request:\n"
              << buffer << console::colors::reset
              << std::endl;
    // std::string response_body = "<html><body><h1>Hello, World!</h1></body></html>";
    std::string response_header = "HTTP/1.1 200 OK"
                                  "\r\nContent-Type: text/html; charset=UTF-8"
                                  "\r\nConnection: close"
                                  "\r\n\r\n"
                                  "Hello, World!";
    nc.send_data(response_header, response_header.size());
    nc.close_s();
}
void server_thread(unet::sock_type type)
{
    unet::Standby sv(8080, type);
    std::cout << "Standby is running on [::]:8080  type:" << type << std::endl;
    sv.set(8080, type);
    sv.accept_s("server.crt", "server.key");
    std::string request;
    sv.recv_data(request, 4096);
    std::cout << console::colors::green << "from " << unet::ip2str(sv.get_addr())
              << "\nReceived request:\n"
              << request << console::colors::reset
              << std::endl;
    std::string response_header = "HTTP/1.1 200 OK"
                                  "\r\nContent-Type: text/html; charset=UTF-8"
                                  "\r\nConnection: close"
                                  "\r\n\r\n"
                                  "Hello, World!";
    sv.send_data(response_header, response_header.size());
    sv.close_s();
    std::cout << "Standby stopped  type:" << type << std::endl;
}
int udp_thread()
{
    unet::udp_core uc;
    uc.set_port(8080, 8081);
    char udp_buf[1024];
    unet::IPaddress udp_addr;
    int ret = uc.recv_data(&udp_addr, udp_buf, 1024);
    if (ret > 0)
    {
        udp_buf[ret] = '\0';
        std::cout << "from " << unet::ip2str(udp_addr) << std::endl;
        std::cout << "Received UDP message: " << udp_buf << std::endl;
    }
    return 0;
}
int main()
{
    unet::netcpp_start();
    { // getaddrinfo
        std::cout << "---- getaddrinfo ----" << std::endl;
        addrinfo hints = {};
        // hints.ai_family = AF_INET;       // IPv4
        hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_flags = AI_PASSIVE;     // For wildcard IP address
        addrinfo *res;
        int status = getaddrinfo("example.com", NULL, &hints, &res);
        if (status != 0)
        {
            // std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
            return 1;
        }
        for (addrinfo *p = res; p != nullptr; p = p->ai_next)
        {
            // char ipstr[INET_ADDRSTRLEN];
            char ipstr[INET6_ADDRSTRLEN];
            // struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            // inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
            void *addr;
            if (p->ai_family == AF_INET) // IPv4
            {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                addr = &(ipv4->sin_addr);
            }
            else // IPv6
            {
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                addr = &(ipv6->sin6_addr);
            }
            inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
            std::cout << "IP Address: " << ipstr << std::endl;
        }
        freeaddrinfo(res);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 100ミリ秒

    { // getipaddeinfo
        std::cout << "---- getipaddrinfo ----" << std::endl;
        const char *addrstr = "example.com";
        int port = 80;
        unet::IPaddress ret;
        if (unet::getipaddrinfo(addrstr, port, ret) == unet::success)
        {
            std::cout << "IP Address: " << unet::ip2str(ret) << std::endl;
            // httpリクエストを送ってみる
            int sock = socket(((struct sockaddr_in *)&ret)->sin_family, SOCK_STREAM, 0);
            // int sock = socket(ret.ai_family, SOCK_STREAM, 0);
            connect(sock, (struct sockaddr *)&ret, sizeof(ret));
            // connect(sock, ret.ai_addr, ret.ai_addrlen);
            const char *http_request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
            send(sock, http_request, strlen(http_request), 0);
            char buffer[4096];
            int bytes_received;
            while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
            {
                buffer[bytes_received] = '\0'; // Null-terminate the received data
                std::cout << buffer;
            }
            close(sock);
        }
        else
        {
            std::cout << "getipaddrinfo error" << std::endl;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 100ミリ秒

    { // ClientTCPipV6
        std::cout << "---- ClientTCPipV6 ----" << std::endl;
        // unet::ClientTCPipV6 client("example.com", 80);
        unet::Client client;
        client.connect_s("example.com", unet::sock_type::SSL_c);
        client.send_data(unet::http::get_http_request_header("GET", "/", "example.com"));
        std::string response = client.recv_all();
        // std::cout << response << std::endl;
        std::cout << unet::http::extract_http_header(response) << std::endl;
        client.close_s();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 100ミリ秒

    { // Server
        std::cout << "---- Server ----" << std::endl;
        unet::Server server(8080, fnc, unet::sock_type::TCP_c, "server.crt", "server.key");
        std::cout << "Server is running on [::]:8080" << std::endl;
        server.listen_p(false); // 非ブロッキングでリッスン開始
        unet::Client client;
        client.connect_s("::1", unet::sock_type::TCP_c, 8080);
        client.send_data(unet::http::get_http_request_header("GET", "/", "localhost"));
        std::string response = client.recv_all();
        client.close_s();
        std::cout << console::colors::green << response << console::colors::reset << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 100ミリ秒

    { // ServerSSL
        std::cout << "---- ServerSSL ----" << std::endl;
        unet::Server server(8080, fnc, unet::sock_type::SSL_c, "server.crt", "server.key");
        std::cout << "Server is running on [::]:8080" << std::endl;
        server.listen_p(false); // 非ブロッキングでリッスン開始
        unet::Client client;
        client.connect_s("::1", unet::sock_type::SSL_c, 8080);
        client.send_data(unet::http::get_http_request_header("GET", "/", "localhost"));
        std::string response = client.recv_all();
        client.close_s();
        std::cout << console::colors::green << response << console::colors::reset << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 100ミリ秒

    { // Standby
        std::cout << "---- Standby ----" << std::endl;
        std::thread th(server_thread, unet::sock_type::TCP_c);
        // std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100ミリ秒
        unet::Standby sv(8080, unet::sock_type::TCP_c);
        sv.set(8080, unet::sock_type::TCP_c);
        sv.connect_s("::1");
        sv.send_data(unet::http::get_http_request_header("GET", "/", "localhost"));
        std::string response = sv.recv_all();
        sv.close_s();
        std::cout << console::colors::blue << response << console::colors::reset << std::endl;
        th.join();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 100ミリ秒

    { // StandbySSL
        std::cout << "---- StandbySSL ----" << std::endl;
        std::thread th(server_thread, unet::sock_type::SSL_c);
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 100ミリ秒
        unet::Standby sv_ssl(8080, unet::sock_type::SSL_c);
        sv_ssl.set(8080, unet::sock_type::SSL_c);
        sv_ssl.connect_s("::1");
        sv_ssl.send_data(unet::http::get_http_request_header("GET", "/", "localhost"));
        std::string response = sv_ssl.recv_all();
        sv_ssl.close_s();
        std::cout << console::colors::blue << response << console::colors::reset << std::endl;
        th.join();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 100ミリ秒

    { // UDP
        std::cout << "---- UDP ----" << std::endl;
        std::thread th(udp_thread);
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 100
        unet::udp_core uc;
        uc.set_port(8081, 8080);
        uc.send_data("UDP Client", "Hello via UDP", 13);
        char udp_buf[1024];
        int udp_bytes = uc.recv_data(udp_buf, sizeof(udp_buf) - 1);
        if (udp_bytes > 0)
        {
            udp_buf[udp_bytes] = '\0';
            std::cout << "Received UDP message: " << udp_buf << std::endl;
        }
        th.join();
    }
    std::cout << "---- Finished ----" << std::endl;

    unet::netcpp_stop();
    return 0;
}