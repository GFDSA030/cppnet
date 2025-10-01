#include <client.h>
#include <infnc.h>
#include <string>
#include <thread>

namespace unet
{
    Client::Client(const char *addr_, const sock_type type_, const int port_) noexcept
    {
        set_type(type_);
        type = type_;
        int port = (port_ == -1) ? (int)type_ : port_;

        if (getipaddrinfo(addr_, port, addr, type_) != success)
        {
            perror("getipaddrinfo failed");
            this_status = offline;
            return;
        }

        sock = socket(addr.ss_family, SOCK_STREAM, 0);
        if (sock < 0)
        {
            perror("socket() failed");
            this_status = offline;
            return;
        }
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("connect() failed");
            close(sock);
            this_status = offline;
            return;
        }

#ifdef NETCPP_SSL_AVAILABLE
        if (type == SSL_c)
        {
            ctx = SSL_CTX_new(TLS_client_method());
            if (!ctx)
            {
                perror("SSL_CTX_new failed");
                close(sock);
                this_status = offline;
                return;
            }
            ssl = SSL_new(ctx);
            // SNIを設定
            SSL_set_tlsext_host_name(ssl, addr_);

            SSL_set_fd(ssl, sock);
            if (SSL_connect(ssl) <= 0)
            {
                perror("SSL_connect failed");
                ERR_print_errors_fp(stderr);
                SSL_free(ssl);
                ssl = nullptr;
                SSL_CTX_free(ctx);
                ctx = nullptr;
                close(sock);
                this_status = offline;
                return;
            }
        }
#else
        if (type_ == SSL_c)
            fprintf(stderr, "ssl isn't avilable\n");
#endif // NETCPP_SSL_AVAILABLE
        this_status = online;
    }

    int Client::connect_s(const char *addr_, const sock_type type_, const int port_) noexcept
    {
        close_m();
        set_type(type_);
        type = type_;
        int port = (port_ == -1) ? (int)type_ : port_;

        if (getipaddrinfo(addr_, port, addr, type_) != success)
        {
            perror("getipaddrinfo failed");
            this_status = offline;
            return error;
        }

        sock = socket(addr.ss_family, SOCK_STREAM, 0);
        if (sock < 0)
        {
            perror("socket() failed");
            this_status = offline;
            return error;
        }
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            perror("connect() failed");
            close(sock);
            this_status = offline;
            return error;
        }

#ifdef NETCPP_SSL_AVAILABLE
        if (type == SSL_c)
        {
            ctx = SSL_CTX_new(TLS_client_method());
            if (!ctx)
            {
                perror("SSL_CTX_new failed");
                close(sock);
                this_status = offline;
                return error;
            }
            ssl = SSL_new(ctx);
            // SNIを設定
            SSL_set_tlsext_host_name(ssl, addr_);

            SSL_set_fd(ssl, sock);
            if (SSL_connect(ssl) <= 0)
            {
                perror("SSL_connect failed");
                ERR_print_errors_fp(stderr);
                SSL_free(ssl);
                ssl = nullptr;
                SSL_CTX_free(ctx);
                ctx = nullptr;
                close(sock);
                this_status = offline;
                return error;
            }
        }
#else
        if (type_ == SSL_c)
            fprintf(stderr, "ssl isn't avilable\n");
#endif // NETCPP_SSL_AVAILABLE
        this_status = online;
        return success;
    }

    sock_type Client::change_type(const sock_type type_) noexcept
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

    Client::Client() noexcept
    {
    }

    Client::~Client()
    {
        close_m();
    }

}
