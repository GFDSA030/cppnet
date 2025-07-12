#include <server.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
#ifdef NETCPP_SSL_AVAILABLE
    ServerSSL::ServerSSL(int port_, void (*fnc_)(net_core &), const char *crt, const char *pem, bool thread_) noexcept
    {
        netcpp_start();
        fnc = fnc_;
        type = SSL_c;
        thread_use = thread_;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            fprintf(stderr, "Error. Cannot make socket\n");
            return;
        }
        const int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            fprintf(stderr, "Error. Cannot bind socket\n");
            return;
        }
        listen(sock, 25);

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
    }

    ServerSSL::ServerSSL(int port_, void (*fnc_)(net_core &), sock_type type_, const char *crt, const char *pem, bool thread_) noexcept
    {
        netcpp_start();
        fnc = fnc_;
        type = SSL_c;
        thread_use = thread_;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            fprintf(stderr, "Error. Cannot make socket\n");
            return;
        }
        const int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            fprintf(stderr, "Error. Cannot bind socket\n");
            return;
        }
        listen(sock, 25);

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
    }

    ServerSSL::~ServerSSL()
    {
    }

    int ServerSSL::listen_m() noexcept
    {
        struct sockaddr_in client;
        uint len;
        int sockcli;
        while (cont == 1)
        {
            len = sizeof(client);
            sockcli = accept(sock, (struct sockaddr *)&client, &len);
#ifndef NETCPP_BLOCKING
            u_long val = 1;
            ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING

            SSL *ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sockcli);
            if (!SSL_accept(ssl))
            {
                perror("Error: SSL_accept()");
                ERR_print_errors_fp(stderr);
                continue;
            }

            run_fn(this, fnc, sockcli, client, SSL_c, ssl, thread_use);
        }
        return success;
    }
    int ServerSSL::listen_p(bool block) noexcept
    {
        if (block)
            return listen_m();
        else
        {
            std::thread t(&ServerSSL::listen_m, this);
            t.detach();
            return success;
        }
    }

#endif
}
