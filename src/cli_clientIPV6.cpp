#include <client.h>
#include <infnc.h>
#include <string>
#include <thread>

namespace unet
{
    ClientIPV6::ClientIPV6(const char *addr_, const sock_type type_, const int port_) noexcept
    {
        set_type(type_);
        getipaddrinfo(addr_, port_, addrV6, type_);
        type = type_;

        sock = socket(addrV6.ai_family, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        connect(sock, addrV6.ai_addr, addrV6.ai_addrlen);

#ifdef NETCPP_SSL_AVAILABLE
        if (type == SSL_c)
        {
            ctx = SSL_CTX_new(TLS_client_method());
            ssl = SSL_new(ctx);
            // SNIを設定
            SSL_set_tlsext_host_name(ssl, addr_);

            SSL_set_fd(ssl, sock);
            SSL_connect(ssl);
        }
#else
        if (type_ == SSL_c)
            fprintf(stderr, "ssl isn't avilable\n");
#endif // NETCPP_SSL_AVAILABLE
        this_status = online;
    }

    int ClientIPV6::connect_s(const char *addr_, const sock_type type_, const int port_) noexcept
    {
        close_m();
        set_type(type_);
        getipaddrinfo(addr_, port_, addrV6, type_);
        type = type_;

        sock = socket(addrV6.ai_family, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        connect(sock, addrV6.ai_addr, addrV6.ai_addrlen);

#ifdef NETCPP_SSL_AVAILABLE
        if (type == SSL_c)
        {
            ctx = SSL_CTX_new(TLS_client_method());
            ssl = SSL_new(ctx);
            // SNIを設定
            SSL_set_tlsext_host_name(ssl, addr_);

            SSL_set_fd(ssl, sock);
            SSL_connect(ssl);
        }
#else
        if (type_ == SSL_c)
            fprintf(stderr, "ssl isn't avilable\n");
#endif // NETCPP_SSL_AVAILABLE
        this_status = online;
        return success;
    }

    sock_type ClientIPV6::change_type(const sock_type type_) noexcept
    {
        if (type < 0 || type_ == unknown)
        {
            fprintf(stderr, "type is unknown\n");
            return type;
        }
        type = type_;
        set_type(type_);
        return type;
    }

    ClientIPV6::ClientIPV6() noexcept
    {
    }

    ClientIPV6::~ClientIPV6()
    {
        close_m();
    }

}
