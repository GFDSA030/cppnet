#include <client.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
#ifdef NETCPP_SSL_AVAILABLE
    ClientSSL::ClientSSL() noexcept
    {
    }

    ClientSSL::ClientSSL(const char *addr_, const int port_) noexcept
    {
        getipaddr(addr_, addr);
        type = SSL_c;
        addr.sin_port = htons(port_);
        addr.sin_family = AF_INET;

        sock = socket(AF_INET, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        connect(sock, (struct sockaddr *)&addr, sizeof(addr));

        ctx = SSL_CTX_new(TLS_client_method());
        ssl = SSL_new(ctx);
        // SNIを設定
        SSL_set_tlsext_host_name(ssl, addr_);

        SSL_set_fd(ssl, sock);
        SSL_connect(ssl);
        this_status = online;
    }

    int ClientSSL::connect_s(const char *addr_, const int port_) noexcept
    {
        getipaddr(addr_, addr);
        type = SSL_c;
        addr.sin_port = htons(port_);
        addr.sin_family = AF_INET;

        sock = socket(AF_INET, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        connect(sock, (struct sockaddr *)&addr, sizeof(addr));

        ctx = SSL_CTX_new(TLS_client_method());
        ssl = SSL_new(ctx);
        // SNIを設定
        SSL_set_tlsext_host_name(ssl, addr_);

        SSL_set_fd(ssl, sock);
        this_status = online;
        return SSL_connect(ssl);
    }

    ClientSSL::~ClientSSL()
    {
        close_m();
    }
#endif // NETCPP_SSL_AVAILABLE
}
