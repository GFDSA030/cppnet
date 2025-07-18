#include <iostream>
#include <unet.h>
using namespace unet;
int main()
{
    std::cout << "===== cppnet テスト開始 =====" << std::endl;

    // base.h
    netinit();
    std::cout << "netinit() 実行" << std::endl;

    // client.h
    // Client_comでTCP通信 (HTTP)
    Client_com client_com("example.com", TCP_c);
    std::cout << "Client_com インスタンス生成 (TCP)" << std::endl;
    if (client_com.connect_s("example.com", TCP_c) == 0)
    {
        std::cout << "Client_com::connect_s() (TCP) 成功" << std::endl;
        std::string http_req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
        client_com.send_data(http_req);
        std::cout << "HTTPリクエスト送信 (TCP)" << std::endl;
        std::string http_resp;
        client_com.recv_data(http_resp, 4096);
        std::cout << "HTTPレスポンス受信 (TCP)\n"
                  << std::endl;
    }
    else
    {
        std::cout << "Client_com::connect_s() (TCP) 失敗" << std::endl;
    }

    // Client_comでSSL通信 (HTTPS)
    client_com.change_type(SSL_c);
    std::cout << "Client_com 通信方式変更 (SSL)" << std::endl;
    if (client_com.connect_s("example.com", SSL_c) == 0)
    {
        std::cout << "Client_com::connect_s() (SSL) 成功" << std::endl;
        std::string https_req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
        client_com.send_data(https_req);
        std::cout << "HTTPSリクエスト送信 (SSL)" << std::endl;
        std::string https_resp;
        client_com.recv_data(https_resp, 4096);
        std::cout << "HTTPSレスポンス受信 (SSL)\n"
                  << std::endl;
    }
    else
    {
        std::cout << "Client_com::connect_s() (SSL) 失敗" << std::endl;
    }

    // example.com へ HTTP通信
    ClientTCP client_tcp("example.com");
    std::cout << "ClientTCP インスタンス生成 (example.com:80)" << std::endl;
    if (client_tcp.connect_s("example.com") == 0)
    {
        std::cout << "ClientTCP::connect_s() 成功" << std::endl;
        std::string http_req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
        client_tcp.send_data(http_req);
        std::cout << "HTTPリクエスト送信" << std::endl;
        std::string http_resp;
        client_tcp.recv_data(http_resp, 4096);
        std::cout << "HTTPレスポンス受信\n"
                  << std::endl;
    }
    else
    {
        std::cout << "ClientTCP::connect_s() 失敗" << std::endl;
    }

    // example.com へ HTTPS通信
    ClientSSL client_ssl("example.com");
    std::cout << "ClientSSL インスタンス生成 (example.com:443)" << std::endl;
    if (client_ssl.connect_s("example.com") == 0)
    {
        std::cout << "ClientSSL::connect_s() 成功" << std::endl;
        std::string https_req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
        client_ssl.send_data(https_req);
        std::cout << "HTTPSリクエスト送信" << std::endl;
        std::string https_resp;
        client_ssl.recv_data(https_resp, 4096);
        std::cout << "HTTPSレスポンス受信\n"
                  << std::endl;
    }
    else
    {
        std::cout << "ClientSSL::connect_s() 失敗" << std::endl;
    }

    // server.h
    // Server_com, ServerSSL, ServerTCP などはコールバック関数が必要

    netquit();
    std::cout << "netquit() 実行" << std::endl;

    std::cout << "===== cppnet テスト終了 =====" << std::endl;
    return 0;
}
