#include <server.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
    ServerIPV6::ServerIPV6(int port_, svrCallbackFn fnc_, sock_type type_, const char *crt, const char *pem, bool thread_) noexcept
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
            fprintf(stderr, "Error. Cannot make socket\n");
            return;
        }
        const int opt = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
        {
            fprintf(stderr, "setsockopt SO_REUSEADDR error\n");
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
            fprintf(stderr, "Error. Cannot bind socket\n");
            return;
        }
        listen(sock, 25);

#ifdef NETCPP_SSL_AVAILABLE
        if (type != SSL_c)
            return;
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
        if (type_ == SSL_c)
            fprintf(stderr, "ssl isn't avilable\n");
#endif
    }

    ServerIPV6::~ServerIPV6()
    {
    }

    int ServerIPV6::listen_m() noexcept
    {
        IPaddress client;
        uint len = sizeof(client);
        int sockcli;
        while (cont == 1)
        {
            sockcli = accept(sock, (struct sockaddr *)&client, &len);
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
                    continue;
                }
            }
#endif

            run_fn(this, fnc, sockcli, client, type, ssl, thread_use, UserData);
        }
        return success;
    }
    int ServerIPV6::listen_p(bool block) noexcept
    {
        if (block)
            return listen_m();
        else
        {
            std::thread t(&ServerIPV6::listen_m, this);
            t.detach();
            return success;
        }
    }

    sock_type ServerIPV6::change_type(const sock_type type_) noexcept
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
