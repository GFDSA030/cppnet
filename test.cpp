#include <iostream>
#include <unet.h>
#include <http.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <bitset>

namespace console
{

    [[maybe_unused]] static const char reset[] = "\033[0m\033[39m\033[49m";
    namespace decoration
    {
        [[maybe_unused]] static const char bold[] = "\033[1m";
        [[maybe_unused]] static const char underline[] = "\033[4m";
        [[maybe_unused]] static const char blink[] = "\033[5m";
        [[maybe_unused]] static const char reversed[] = "\033[7m";
        [[maybe_unused]] static const char reset[] = "\033[0m";
    }
    namespace colors
    {
        [[maybe_unused]] static const char black[] = "\033[30m";
        [[maybe_unused]] static const char red[] = "\033[31m";
        [[maybe_unused]] static const char green[] = "\033[32m";
        [[maybe_unused]] static const char yellow[] = "\033[33m";
        [[maybe_unused]] static const char blue[] = "\033[34m";
        [[maybe_unused]] static const char masenda[] = "\033[35m";
        [[maybe_unused]] static const char sian[] = "\033[36m";
        [[maybe_unused]] static const char white[] = "\033[37m";
        [[maybe_unused]] static const char reset[] = "\033[39m";
    }
    namespace bg_colors
    {
        [[maybe_unused]] static const char black[] = "\033[40m";
        [[maybe_unused]] static const char red[] = "\033[41m";
        [[maybe_unused]] static const char green[] = "\033[42m";
        [[maybe_unused]] static const char yellow[] = "\033[43m";
        [[maybe_unused]] static const char blue[] = "\033[44m";
        [[maybe_unused]] static const char masenda[] = "\033[45m";
        [[maybe_unused]] static const char sian[] = "\033[46m";
        [[maybe_unused]] static const char gray[] = "\033[47m";
        [[maybe_unused]] static const char reset[] = "\033[49m";
    }
} // namespace hds::console

void fnc(unet::net_core &nc, void *udata)
{
    // std::string request = nc.recv_all();
    char buffer[4096];
    [[maybe_unused]] int bytes_received = nc.recv_data(buffer, sizeof(buffer) - 1);
    std::cout << console::colors::red << "from " << unet::ip2str(nc.remote())
              << "\nReceived request:\n"
              << buffer << console::colors::reset
              << "\n";
    std::string response_header = "HTTP/1.1 200 OK"
                                  "\r\nContent-Type: text/html; charset=UTF-8"
                                  "\r\nConnection: close"
                                  "\r\n\r\n"
                                  "Hello, World!";
    nc.send_data(response_header, response_header.size());
    nc.close_s();
}
void Standby_thread(unet::sock_type type, int p)
{
    unet::Standby sv(p, type);
    std::cout << "Standby is running on [::]:" << p << "  type:" << type << "\n";
    sv.set(p, type);
    int accept_result = sv.accept_s("server.crt", "server.key");
    if (accept_result != unet::success)
    {
        std::cout << console::colors::red << "accept_s failed" << console::colors::reset << "\n";
        return;
    }
    std::cout << "accept_s succeeded, waiting for data..." << "\n";
    std::string request;
    int recv_result = sv.recv_data(request, 4096);
    if (recv_result <= 0)
    {
        std::cout << console::colors::red << "recv_data failed or no data" << console::colors::reset << "\n";
        sv.close_s();
        return;
    }
    std::cout << console::colors::green << "from " << unet::ip2str(sv.get_addr())
              << "\nReceived request:\n"
              << request << console::colors::reset
              << "\n";
    std::string response_header = "HTTP/1.1 200 OK"
                                  "\r\nContent-Type: text/html; charset=UTF-8"
                                  "\r\nConnection: close"
                                  "\r\n\r\n"
                                  "Hello, World!";
    sv.send_data(response_header, response_header.size());
    sv.close_s();
    std::cout << "Standby stopped  type:" << type << "\n";
}
void udp_thread()
{
    unet::udp_core uc;
    uc.set_port(8080, 8081);
    char udp_buf[1024] = {0};
    unet::IPaddress udp_addr;
    uc.recv_data(&udp_addr, udp_buf, 1024);
    std::cout << console::colors::masenda
              << "from " << unet::ip2str(udp_addr) << "\nReceived UDP message: " << udp_buf
              << console::colors::reset << "\n";
    uc.send_data(unet::ip2str(udp_addr).c_str(), udp_buf, sizeof(udp_buf));
}
int main()
{
    constexpr int test_delay = 1;
    constexpr int server_delay = 5;

    static uint32_t results = 0;

    std::vector<std::pair<std::string, int>> result;

    unet::netcpp_start();
    do
    { // getaddrinfo
        std::cout << "---- getaddrinfo To:[example.com] ----" << "\n";
        addrinfo hints = {};
        // hints.ai_family = AF_INET;       // IPv4
        hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_flags = AI_PASSIVE;     // For wildcard IP address
        addrinfo *res;
        int status = getaddrinfo("example.com", NULL, &hints, &res);
        if (status != 0)
        {
            result.push_back({"---- getaddrinfo To:[example.com] ----", unet::error});
            break;
        }
        for (addrinfo *p = res; p != nullptr; p = p->ai_next)
        {
            // char ipstr[INET_ADDRSTRLEN];
            char ipstr[INET6_ADDRSTRLEN];
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
            std::cout << "IP Address: " << ipstr << "\n";
        }
        freeaddrinfo(res);
        results |= 1 << 0;
        result.push_back({"---- getaddrinfo To:[example.com] ----", unet::success});
    } while (0);
    std::this_thread::sleep_for(std::chrono::milliseconds(test_delay));

    { // getipaddrinfo
        std::cout << "---- getipaddrinfo To:[example.com] ----" << "\n";
        const char *addrstr = "example.com";
        int port = 80;
        unet::IPaddress ret;
        if (unet::getipaddrinfo(addrstr, port, ret) == unet::success)
        {
            std::cout << "IP Address: " << unet::ip2str(ret) << "\n";
            // httpリクエストを送ってみる
            int sock = socket(((struct sockaddr_in *)&ret)->sin_family, SOCK_STREAM, 0);
            connect(sock, (struct sockaddr *)&ret, sizeof(ret));
            // connect(sock, ret.ai_addr, ret.ai_addrlen);
            const char *http_request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
            send(sock, http_request, strlen(http_request), 0);
            char buffer[4096];
            int bytes_received;
            while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
            {
                buffer[bytes_received] = '\0'; // Null-terminate the received data
                std::cout << console::colors::green << buffer;
            }
            close(sock);
            results |= 1 << 1;
            result.push_back({"---- getipaddrinfo To:[example.com] ----", unet::success});
        }
        else
        {
            std::cout << "getipaddrinfo error" << "\n";
            result.push_back({"---- getipaddrinfo To:[example.com] ----", unet::error});
        }
        std::cout << console::colors::reset;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(test_delay));

    { // Client SSL_c
        std::cout << "---- Client SSL_c To:[example.com] ----" << "\n";
        unet::Client client;
        client.connect_s("example.com", unet::sock_type::SSL_c);
        client.send_data(unet::http::get_http_request_header("GET", "/", "example.com"));
        std::string response = client.recv_all();
        std::cout << console::colors::green << unet::http::extract_http_header(response) << console::colors::reset << "\n";
        client.close_s();
        if (response.starts_with("HTTP/1.1 200 OK"))
        {
            result.push_back({"---- Client SSL_c To:[example.com] ----", unet::success});
            results |= 1 << 2;
        }
        else
        {
            result.push_back({"---- Client SSL_c To:[example.com] ----", unet::error});
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(test_delay));

    { // Server TCP_c
        std::cout << "---- Server TCP_c To:[::1] ----" << "\n";
        unet::Server server(8080, fnc, unet::sock_type::TCP_c, "server.crt", "server.key");
        std::cout << "Server is running on [::]:8080" << "\n";
        server.listen_p(false); // 非ブロッキングでリッスン開始
        unet::Client client;
        client.connect_s("::1", unet::sock_type::TCP_c, 8080);
        client.send_data(unet::http::get_http_request_header("GET", "/", "localhost"));
        std::string response = client.recv_all();
        client.close_s();
        std::cout << console::colors::green << response << console::colors::reset << "\n";
        if (response.starts_with("HTTP/1.1 200 OK"))
        {
            result.push_back({"---- Server TCP_c To:[::1] ----", unet::success});
            results |= 1 << 3;
        }
        else
        {
            result.push_back({"---- Server TCP_c To:[::1] ----", unet::error});
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(test_delay));

    { // Server SSL_c
        std::cout << "---- Server SSL_c To:[::1] ----" << "\n";
        unet::Server server(8088, fnc, unet::sock_type::SSL_c, "server.crt", "server.key");
        std::cout << "Server is running on [::]:8080" << "\n";
        server.listen_p(false); // 非ブロッキングでリッスン開始
        unet::Client client;
        client.connect_s("::1", unet::sock_type::SSL_c, 8088);
        client.send_data(unet::http::get_http_request_header("GET", "/", "localhost"));
        std::string response = client.recv_all();
        client.close_s();
        std::cout << console::colors::green << response << console::colors::reset << "\n";
        if (response.starts_with("HTTP/1.1 200 OK"))
        {
            result.push_back({"---- Server SSL_c To:[::1] ----", unet::success});
            results |= 1 << 4;
        }
        else
        {
            result.push_back({"---- Server SSL_c To:[::1] ----", unet::error});
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(test_delay));

    { // Standby Client TCP_c
        std::cout << "---- Standby Client TCP_c To:[example.com] ----" << "\n";
        unet::Standby sv(80, unet::sock_type::TCP_c);
        sv.set(80, unet::sock_type::TCP_c);
        sv.connect_s("example.com");
        sv.send_data(unet::http::get_http_request_header("GET", "/", "example.com"));
        std::string response = sv.recv_all();
        sv.close_s();
        std::cout << console::colors::blue << response << console::colors::reset << "\n";
        if (response.starts_with("HTTP/1.1 200 OK"))
        {
            result.push_back({"---- Standby Client TCP_c To:[::1] ----", unet::success});
            results |= 1 << 5;
        }
        else
        {
            result.push_back({"---- Standby Client TCP_c To:[::1] ----", unet::error});
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(test_delay));

    { // Standby Client SSL_c
        std::cout << "---- Standby Client SSL_c To:[example.com] ----" << "\n";
        unet::Standby sv(443, unet::sock_type::SSL_c);
        sv.set(443, unet::sock_type::SSL_c);
        sv.connect_s("example.com");
        sv.send_data(unet::http::get_http_request_header("GET", "/", "example.com"));
        std::string response = sv.recv_all();
        sv.close_s();
        std::cout << console::colors::blue << response << console::colors::reset << "\n";
        if (response.starts_with("HTTP/1.1 200 OK"))
        {
            result.push_back({"---- Standby Client SSL_c To:[::1] ----", unet::success});
            results |= 1 << 6;
        }
        else
        {
            result.push_back({"---- Standby Client SSL_c To:[::1] ----", unet::error});
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(test_delay));
    { // UDP
        std::cout << "---- UDP To:[::1] ----" << "\n";
        std::thread th(udp_thread);
        std::this_thread::sleep_for(std::chrono::milliseconds(server_delay)); // 100
        unet::udp_core uc;
        uc.set_port(8081, 8080);
        uc.send_data("::1", "Hello via UDP", 13);
        char udp_buf[1024] = {0};
        int udp_bytes = uc.recv_data(udp_buf, sizeof(udp_buf) - 1);
        if (udp_bytes > 0)
        {
            udp_buf[udp_bytes] = '\0';
            std::cout << "Received UDP message: " << udp_buf << "\n";
        }
        th.join();
        if (std::string(udp_buf).starts_with("Hello via UDP"))
        {
            result.push_back({"---- UDP To:[::1] ----", unet::success});
            results |= 1 << 7;
        }
        else
        {
            result.push_back({"---- UDP To:[::1] ----", unet::error});
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(test_delay));
    { // Standby Server TCP_c
        std::cout << "---- Standby Server TCP_c To:[::1] ----" << "\n";
        std::thread th(Standby_thread, unet::sock_type::TCP_c, 9090);
        std::this_thread::sleep_for(std::chrono::milliseconds(server_delay)); // サーバーの完全な起動待機
        unet::Standby sv(9090, unet::sock_type::TCP_c);
        sv.set(9090, unet::sock_type::TCP_c);
        int connect_result = sv.connect_s("::1");
        if (connect_result != unet::success)
        {
            std::cout << console::colors::red << "connect_s failed" << console::colors::reset << "\n";
            result.push_back({"---- Standby Server TCP_c To:[::1] ----", unet::error});
        }
        else
        {
            sv.send_data(unet::http::get_http_request_header("GET", "/", "localhost"));
            std::string response = sv.recv_all();
            sv.close_s();
            std::cout << console::colors::blue << response << console::colors::reset << "\n";
            if (response.starts_with("HTTP/1.1 200 OK"))
            {
                result.push_back({"---- Standby Server TCP_c To:[::1] ----", unet::success});
                results |= 1 << 8;
            }
            else
            {
                result.push_back({"---- Standby Server TCP_c To:[::1] ----", unet::error});
            }
        }
        th.join();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(test_delay));

    { // Standby Server SSL_c
        std::cout << "---- Standby Server SSL_c To:[::1] ----" << "\n";
        std::thread th(Standby_thread, unet::sock_type::SSL_c, 7070);
        std::this_thread::sleep_for(std::chrono::milliseconds(server_delay)); // サーバーの完全な起動待機
        unet::Standby sv_ssl(7070, unet::sock_type::SSL_c);
        sv_ssl.set(7070, unet::sock_type::SSL_c);
        int connect_result = sv_ssl.connect_s("::1");
        if (connect_result != unet::success)
        {
            std::cout << console::colors::red << "connect_s failed" << console::colors::reset << "\n";
            result.push_back({"---- Standby Server SSL_c To:[::1] ----", unet::error});
        }
        else
        {
            sv_ssl.send_data(unet::http::get_http_request_header("GET", "/", "localhost"));
            std::string response = sv_ssl.recv_all();
            sv_ssl.close_s();
            std::cout << console::colors::blue << response << console::colors::reset << "\n";
            if (response.starts_with("HTTP/1.1 200 OK"))
            {
                result.push_back({"---- Standby Server SSL_c To:[::1] ----", unet::success});
                results |= 1 << 9;
            }
            else
            {
                result.push_back({"---- Standby Server SSL_c To:[::1] ----", unet::error});
            }
        }
        th.join();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(test_delay));

    std::cout << "---- Finished ----" << "\n";
    for (size_t i = 0; i < result.size(); i++)
    {
        if (result[i].second == unet::success)
        {
            std::cout << console::colors::green << result[i].first << console::reset << "  Success" << std::endl;
        }
        else
        {
            std::cout << console::colors::red << result[i].first << console::reset << "  Faild" << std::endl;
        }
    }
    std::cout << console::colors::green << std::bitset<10>(results) << console::reset << std::endl;
    std::cout << unet::ip2str(unet::getipaddrinfo("localhost", 80)) << std::endl;
    unet::netcpp_stop();
    return 0;
}
