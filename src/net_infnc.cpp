#include <infnc.h>
#include <unordered_map>

void DEBUG_PRINT()
{
    static uint32_t testno = 0;
    printf("file:%s  line:%d  no:%u  \n", __FILE__, __LINE__, testno++);
}

namespace unet
{
#ifdef NETCPP_SSL_AVAILABLE

#if defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER)

    WSADATA data;
    void netinit() noexcept
    {
        netcpp_setstatus(online);
        WSAStartup(MAKEWORD(2, 2), &data);
        SSL_load_error_strings();
        SSL_library_init();
    }
    void netquit() noexcept
    {
        ERR_free_strings();
        WSACleanup();
        netcpp_setstatus(offline);
    }

#else // not Windows

    void netinit() noexcept
    {
        netcpp_setstatus(online);
        SSL_load_error_strings();
        SSL_library_init();
    }
    void netquit() noexcept
    {
        ERR_free_strings();
        netcpp_setstatus(offline);
    }

#endif // windows check

#else // NETCPP_SSL_AVAILABLE

#if defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER)

    WSADATA data;
    void netinit() noexcept
    {
        netcpp_setstatus(online);
        WSAStartup(MAKEWORD(2, 0), &data);
    }
    void netquit() noexcept
    {
        WSACleanup();
        netcpp_setstatus(offline);
    }

#else // not Windows

    void netinit() noexcept
    {
        netcpp_setstatus(online);
    }
    void netquit() noexcept
    {
        netcpp_setstatus(offline);
    }

#endif // windows check

#endif // NETCPP_SSL_AVAILABLE

    infnc::infnc()
    {
        netcpp_start();
    }

    infnc::~infnc()
    {
        netcpp_stop();
    }
    infnc net;

    int Def_connect(int &sock, sock_type &type, status &this_status, int &port, IPaddress &addr, const char *addr_, SSL *ssl, SSL_CTX *ctx) noexcept
    {
        if (getipaddrinfo(addr_, port, addr, type) != success)
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
        switch (type)
        {
        case TCP_c:
        {
            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
            {
                perror("connect() failed");
                close(sock);
                this_status = offline;
                return error;
            }
        }
        break;
        case CRY_c:
        {
            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
            {
                perror("connect() failed");
                close(sock);
                this_status = offline;
                return error;
            }
        }

        break;
        case SSL_c:
        {
            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
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
        }
        break;

        default:
            break;
        }
        return success;
    }

} // namespace unet
