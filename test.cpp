#include <iostream>
#include <unet.h>
#include <http.h>
#include <unistd.h>
#include <chrono>
#include <thread>
void fnc(unet::net_core &nc, void *udata)
{
    // std::string request = nc.recv_all();
    char buffer[4096];
    int bytes_received = nc.recv_data(buffer, sizeof(buffer) - 1);
    std::cout << "from " << unet::ip2str(nc.remote())
              << "\nReceived request:\n"
              << buffer << std::endl;
    // std::string response_body = "<html><body><h1>Hello, World!</h1></body></html>";
    std::string response_header = "HTTP/1.1 200 OK"
                                  "\r\nContent-Type: text/html; charset=UTF-8"
                                  "\r\nConnection: close"
                                  "\r\n\r\n"
                                  "Hello, World!";
    nc.send_data(response_header, response_header.size());
    nc.close_s();
}
int main()
{
    unet::netcpp_start();
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

    std::cout << "---- ClientTCPipV6 ----" << std::endl;
    // unet::ClientTCPipV6 client("example.com", 80);
    unet::Client client;
    client.connect_s("example.com", unet::sock_type::SSL_c);
    client.send_data(unet::http::get_http_request_header("GET", "/", "example.com"));
    std::string response = client.recv_all();
    // std::cout << response << std::endl;
    std::cout << unet::http::extract_http_body(response) << std::endl;
    client.close_s();

    std::cout << "---- ServerIPV6 ----" << std::endl;
    unet::Server server(8080, fnc, unet::sock_type::TCP_c);
    std::cout << "Server is running on [::]:8080" << std::endl;
    server.listen_p();                                     // 非ブロッキングでリッスン開始
    std::this_thread::sleep_for(std::chrono::seconds(60)); // 60秒後にサーバを停止
    server.stop();

    unet::netcpp_stop();
    return 0;
}