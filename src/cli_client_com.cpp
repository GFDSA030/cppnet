#include <client.h>
#include <infnc.h>
#include <string>
#include <thread>

namespace unet
{
    Client_com::Client_com(const char *addr_, const sock_type type_, const int port_) noexcept
    {
        getipaddr(addr_, addr);
        addr.sin_port = htons(port_);
        if (port_ == -1)
            addr.sin_port = htons(type_);
        addr.sin_family = AF_INET;
        type = type_;

        sock = socket(AF_INET, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        connect(sock, (struct sockaddr *)&addr, sizeof(addr));

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

    int Client_com::connect_s(const char *addr_, const sock_type type_, const int port_) noexcept
    {
        getipaddr(addr_, addr);
        addr.sin_port = htons(port_);
        if (port_ == -1)
            addr.sin_port = htons(type_);
        addr.sin_family = AF_INET;
        type = type_;

        sock = socket(AF_INET, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        int ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));

#ifdef NETCPP_SSL_AVAILABLE
        if (type == SSL_c)
        {
            ctx = SSL_CTX_new(TLS_client_method());
            ssl = SSL_new(ctx);
            // SNIを設定
            SSL_set_tlsext_host_name(ssl, addr_);

            SSL_set_fd(ssl, sock);
            ret = SSL_connect(ssl);
        }
#else
        if (type_ == SSL_c)
            fprintf(stderr, "ssl isn't avilable\n");
#endif // NETCPP_SSL_AVAILABLE
        this_status = online;
        return ret;
    }

    sock_type Client_com::change_type(const sock_type type_) noexcept
    {
        if (type < 0 || type_ == unknown)
        {
            fprintf(stderr, "type is unknown\n");
            return type;
        }
        type = type_;
        return type;
    }

    Client_com::Client_com() noexcept
    {
    }

    Client_com::~Client_com()
    {
        close_m();
    }

}
