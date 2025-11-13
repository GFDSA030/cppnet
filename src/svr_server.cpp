#include <server.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
    Server::Server(int port_, svrCallbackFn fnc_, sock_type type_, const char *crt, const char *pem, bool thread_) noexcept
    {
        netcpp_start();
        fnc = fnc_;
        type = type_;
        thread_use = thread_;
        addr.ss_family = AF_INET6;
        ((struct sockaddr_in6 *)&addr)->sin6_addr = in6addr_any;
        ((struct sockaddr_in6 *)&addr)->sin6_port = htons(port_);

        if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
        {
            perror("Error. Cannot make socket");
            return;
        }
        const int opt = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
        {
            perror("setsockopt SO_REUSEADDR error");
            return;
        }
        int off = 0;
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
                       (char *)&off, sizeof(off)) < 0)
        {
            perror("setsockopt IPV6_V6ONLY");
        }

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("Error. Cannot bind socket");
            return;
        }
        if (listen(sock, 25) < 0)
        {
            perror("Error. Cannot listen socket");
            close(sock);
            sock = 0;
            return;
        }

#ifdef NETCPP_SSL_AVAILABLE
        if (type == SSL_c)
        {
            ctx = SSL_CTX_new(TLS_server_method());
            if (!ctx)
            {
                perror("Error: SSL context\n");
                ERR_print_errors_fp(stderr);
                return;
            }
            // load crt
            if (!SSL_CTX_use_certificate_file(ctx, crt, SSL_FILETYPE_PEM))
            {
                perror("Error: SSL_CTX_use_certificate_file()\n");
                ERR_print_errors_fp(stderr);
                return;
            }
            // load private key
            if (!SSL_CTX_use_PrivateKey_file(ctx, pem, SSL_FILETYPE_PEM))
            {
                perror("Error: SSL_CTX_use_PrivateKey_file()\n");
                ERR_print_errors_fp(stderr);
                return;
            }
#else
        fprintf(stderr, "ssl isn't avilable\n");
#endif
        }
    }

    Server::~Server()
    {
    }

    int Server::listen_m() noexcept
    {
        IPaddress client;
        socklen_t len = sizeof(client);
        int sockcli;
        while (cont == true)
        {
#ifdef __MINGW32__
            sockcli = accept(sock, (struct sockaddr *)&client, (int *)&len);
#else
        sockcli = accept(sock, (struct sockaddr *)&client, &len);
#endif
            if (sockcli < 0)
            {
                perror("accept() failed");
                continue;
            }
#ifndef NETCPP_BLOCKING
            u_long val = 1;
            ioctl(sockcli, FIONBIO, &val);
#endif // NETCPP_BLOCKING

            SSL *ssl = nullptr;
#ifdef NETCPP_SSL_AVAILABLE
            if (type == SSL_c)
            {
                ssl = SSL_new(ctx);
                SSL_set_fd(ssl, sockcli);
                if (!SSL_accept(ssl))
                {
                    perror("Error: SSL_accept()");
                    ERR_print_errors_fp(stderr);
                    SSL_free(ssl);
                    ssl = nullptr;
                    close(sockcli);
                    continue;
                }
            }
#endif

            run_fn(this, fnc, sockcli, client, type, ssl, thread_use, UserData);
        }
        return success;
    }
    int Server::listen_p(bool block) noexcept
    {
        if (block)
            return listen_m();
        else
        {
            std::thread t(&Server::listen_m, this);
            t.detach();
            return success;
        }
    }

    sock_type Server::change_type(const sock_type type_) noexcept
    {
        if (type < 0 || type_ == unknown)
        {
            fprintf(stderr, "type is unknown\n");
            return type;
        }
        type = type_;
        return type;
    }
}
