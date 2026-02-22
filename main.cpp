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
    con.send_data((send_buf).c_str(), send_buf.size());
    con.close_s();
}

int main()
{

    unet::Server svr(9090, get, unet::TCP_c, "server.crt", "server.key");
    svr.listen_p(0);
    while (1)
    {
        std::cout << "\rWaiting for connections..." << svr.get_connection_len() << "\t  \t" << svr.get_connection_no()<<"        ";
        // wait for server
        usleep(1);
    }
}
