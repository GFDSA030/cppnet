#include <base.h>
#include <infnc.h>
namespace unet
{

    Standby::Standby(int port_, const sock_type type_) noexcept
    {
        port = port_;
        change_type(type_);
    }
    Standby::~Standby()
    {
    }
    int Standby::set(int port_, const sock_type type_) noexcept
    {
        port = port_;
        change_type(type_);
        return success;
    }
    int Standby::accept_s(const char *crt, const char *pem) noexcept
    {
        close_s();

        if ((svScok = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            fprintf(stderr, "Error. Cannot make socket\n");
            return error;
        }
        const int opt = 1;
        setsockopt(svScok, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (bind(svScok, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            fprintf(stderr, "Error. Cannot bind socket\n");
            return error;
        }
        listen(svScok, 25);

#ifdef NETCPP_SSL_AVAILABLE
        if (type != SSL_c)
            return success;
        ctx = SSL_CTX_new(TLS_server_method());
        if (!ctx)
        {
            perror("Error: SSL context\n");
            ERR_print_errors_fp(stderr);
            return error;
        }
        // load crt
        if (!SSL_CTX_use_certificate_file(ctx, crt, SSL_FILETYPE_PEM))
        {
            perror("Error: SSL_CTX_use_certificate_file()\n");
            ERR_print_errors_fp(stderr);
            return error;
        }
        // load private key
        if (!SSL_CTX_use_PrivateKey_file(ctx, pem, SSL_FILETYPE_PEM))
        {
            perror("Error: SSL_CTX_use_PrivateKey_file()\n");
            ERR_print_errors_fp(stderr);
            return error;
        }
#else
        if (type_ == SSL_c)
        {
            fprintf(stderr, "ssl isn't avilable\n");
            return error;
        }
#endif
        uint len = sizeof(addr);
        sock = accept(svScok, (struct sockaddr *)&addr, &len);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif
        SSL *ssl = nullptr;
#ifdef NETCPP_SSL_AVAILABLE
        if (type == SSL_c)
        {
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock);
            if (!SSL_accept(ssl))
            {
                perror("Error: SSL_accept()");
                ERR_print_errors_fp(stderr);
                return error;
            }
        }
#endif

        return success;
    }
    int Standby::connect_s(const char *addr_) noexcept
    {
        close_s();
        getipaddr(addr_, addr);
        addr.sin_port = htons(port);
        if (port == -1)
            addr.sin_port = htons(port);
        addr.sin_family = AF_INET;

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
        {
            fprintf(stderr, "ssl isn't avilable\n");
            return error;
        }
#endif // NETCPP_SSL_AVAILABLE
        this_status = online;
        return success;
    }
    int Standby::close_s() noexcept
    {
        close_m();
        if (svScok != 0)
        {
            close(svScok);
        }
        this_status = offline;
        return success;
    }
    sock_type Standby::change_type(const sock_type type_) noexcept
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
} // namespace unet
