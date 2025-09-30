#include <client.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
#ifdef NETCPP_SSL_AVAILABLE
    ClientSSLipV6::ClientSSLipV6() noexcept
    {
    }

    ClientSSLipV6::ClientSSLipV6(const char *addr_, const int port_) noexcept
    {
        set_type(SSL_c);
        getipaddrinfo(addr_, port_, addrV6, SSL_c);
        type = SSL_c;

        sock = socket(addrV6.ai_family, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        connect(sock, addrV6.ai_addr, addrV6.ai_addrlen);

        ctx = SSL_CTX_new(TLS_client_method());
        ssl = SSL_new(ctx);
        // SNIを設定
        SSL_set_tlsext_host_name(ssl, addr_);

        SSL_set_fd(ssl, sock);
        SSL_connect(ssl);
        this_status = online;
    }

    int ClientSSLipV6::connect_s(const char *addr_, const int port_) noexcept
    {
        close_m();
        set_type(SSL_c);
        getipaddrinfo(addr_, port_, addrV6, SSL_c);
        type = SSL_c;

        sock = socket(addrV6.ai_family, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        connect(sock, addrV6.ai_addr, addrV6.ai_addrlen);

        ctx = SSL_CTX_new(TLS_client_method());
        ssl = SSL_new(ctx);
        // SNIを設定
        SSL_set_tlsext_host_name(ssl, addr_);

        SSL_set_fd(ssl, sock);
        this_status = online;
        SSL_connect(ssl);
        return success;
    }

    ClientSSLipV6::~ClientSSLipV6()
    {
        close_m();
    }
#endif // NETCPP_SSL_AVAILABLE
}
