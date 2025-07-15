#include <unet.h>
#include <http.h>
#include <iostream>
#include <string>

void get(unet::net_core &con, void *Udata) noexcept
{
    static size_t no = 0;
    char recv_buf[2048 * 4];
    memset(recv_buf, 0, 2048);
    no++;
    std::string send_buf = std::string("HTTP/1.1 200 OK\r\n") +
                           std::string("Set-Cookie: Sample_acc=accNo") +
                           std::to_string(no).c_str() +
                           std::string("\r\n") +
                           std::string("\r\n") +
                           std::string("<h1>hello</h1>") +
                           std::to_string(no).c_str();

    con.recv_data(recv_buf, 2048 * 4);
    // printf("%s \n%s\n", inet_ntoa(con.remote().sin_addr), recv_buf);
    con.send_data((send_buf).c_str(), send_buf.size());
    con.close_s();
}

int main()
{
    // UDP
    unet::UDP udp;
    udp.set_port(9090, 9090);
    udp.send_data("127.0.0.1", "Hello UDP", 10);
    char udp_buf[1024];
    struct sockaddr_in udp_addr;
    udp.recv_data(&udp_addr, udp_buf, 1024);
    std::cout << "UDP received from " << inet_ntoa(udp_addr.sin_addr) << ": " << udp_buf << std::endl;
    // Server

    unet::Server svr(9090, get, unet::TCP_c, "server.crt", "server.key");
    svr.listen_p(0);

    // Client_com
    std::string target = "example.com";
    unet::Client cli(target.c_str(), unet::SSL_c);
    cli.send_data(unet::get_http_request_header("GET", "/", target));

    // std::cout << cli.recv_all();
    std::cout << unet::extract_http_body(cli.recv_all()) << std::endl;
    while (1)
    {
        std::cout << "\rWaiting for connections..." << svr.get_connection_len() << " " << svr.get_connection_no();
        // wait for server
        usleep(1);
    }
}
