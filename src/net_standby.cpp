#include <base.h>
#include <infnc.h>
namespace unet
{

    Standby::Standby(int port_, const sock_type type_) noexcept
    {
        netcpp_start();
        port = port_;
        change_type(type_);
    }
    Standby::~Standby()
    {
        close_s();
        netcpp_stop();
    }
    int Standby::set(int port_, const sock_type type_) noexcept
    {
        port = port_;
        change_type(type_);
        return success;
    }
    int Standby::accept_s(const char *crt, const char *pem) noexcept
    {
        IPaddress svaddr = {};
        close_s();

        const int opt = 1;
        // Try IPv6 socket first
        svScok = socket(AF_INET6, SOCK_STREAM, 0);
        if (svScok < 0)
        {
            perror("Error. Cannot create IPv6 socket");
            // try IPv4 as fallback
            svScok = socket(AF_INET, SOCK_STREAM, 0);
            if (svScok < 0)
            {
                perror("Error. Cannot create IPv4 socket either");
                return error;
            }
            // configure IPv4 socket
            if (setsockopt(svScok, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
            {
                perror("setsockopt SO_REUSEADDR error (ipv4)");
                close(svScok);
                svScok = 0;
                return error;
            }
            // bind IPv4 ANY
            struct sockaddr_in sv4 = {};
            sv4.sin_family = AF_INET;
            sv4.sin_addr.s_addr = INADDR_ANY;
            sv4.sin_port = htons(port);
            if (bind(svScok, (struct sockaddr *)&sv4, sizeof(sv4)) < 0)
            {
                perror("Error. Cannot bind IPv4 socket");
                close(svScok);
                svScok = 0;
                return error;
            }
        }
        else
        {
            // configure IPv6 socket
            if (setsockopt(svScok, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
            {
                perror("setsockopt SO_REUSEADDR error (ipv6)");
                close(svScok);
                svScok = 0;
                return error;
            }
            int off = 1; // set IPv6-only to avoid conflicts with IPv4 binds
            if (setsockopt(svScok, IPPROTO_IPV6, IPV6_V6ONLY,
                           (char *)&off, sizeof(off)) < 0)
            {
                perror("setsockopt IPV6_V6ONLY");
                // non-fatal, continue
            }

            svaddr.ss_family = AF_INET6;
            ((struct sockaddr_in6 *)&svaddr)->sin6_addr = in6addr_any;
            ((struct sockaddr_in6 *)&svaddr)->sin6_port = htons(port);

            if (bind(svScok, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_in6)) < 0)
            {
                perror("Error. Cannot bind IPv6 socket");
                close(svScok);
                svScok = 0;
                // fallback: try IPv4
                svScok = socket(AF_INET, SOCK_STREAM, 0);
                if (svScok < 0)
                {
                    perror("Error. Cannot create IPv4 socket (fallback)");
                    return error;
                }
                if (setsockopt(svScok, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
                {
                    perror("setsockopt SO_REUSEADDR error (ipv4 fallback)");
                    close(svScok);
                    svScok = 0;
                    return error;
                }
                struct sockaddr_in sv4 = {};
                sv4.sin_family = AF_INET;
                sv4.sin_addr.s_addr = INADDR_ANY;
                sv4.sin_port = htons(port);
                if (bind(svScok, (struct sockaddr *)&sv4, sizeof(sv4)) < 0)
                {
                    perror("Error. Cannot bind IPv4 socket (fallback)");
                    close(svScok);
                    svScok = 0;
                    return error;
                }
            }
        }
        // listen(svScok, 25);
        if (listen(svScok, 25) < 0)
        {
            perror("Error. Cannot listen socket");
            close(svScok);
            svScok = 0;
            return error;
        }

#ifdef NETCPP_SSL_AVAILABLE
        // If server is configured for SSL, initialize SSL context and load cert/key
        if (type == SSL_c)
        {
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
                SSL_CTX_free(ctx);
                ctx = nullptr;
                return error;
            }
            // load private key
            if (!SSL_CTX_use_PrivateKey_file(ctx, pem, SSL_FILETYPE_PEM))
            {
                perror("Error: SSL_CTX_use_PrivateKey_file()\n");
                ERR_print_errors_fp(stderr);
                SSL_CTX_free(ctx);
                ctx = nullptr;
                return error;
            }
        }
#else
        // SSL not available
        if (type == SSL_c)
        {
            fprintf(stderr, "ssl isn't avilable\n");
            return error;
        }
#endif
        socklen_t len = sizeof(addr);
        sock = accept(svScok, (struct sockaddr *)&addr, &len);
        if (sock < 0)
        {
            perror("Error. Cannot accept socket");
            // don't close svScok here, caller may retry accept
            return error;
        }
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
                SSL_free(ssl);
                ssl = nullptr;
                return error;
            }
            // store accepted ssl into object for later send/recv
            this->ssl = ssl;
        }
#endif
        this_status = online;
        return success;
    }
    int Standby::connect_s(const char *addr_) noexcept
    {
        close_s();
        // getipaddrinfo expects (addr, port, ret, type)
        getipaddrinfo(addr_, port, addr, type);

        sock = socket(addr.ss_family, SOCK_STREAM, 0);
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
        if (type == SSL_c)
        {
            fprintf(stderr, "ssl isn't avilable\n");
            return error;
        }
#endif // NETCPP_SSL_AVAILABLE
        this_status = online;
        return success;
    }
    IPaddress Standby::get_addr() const noexcept
    {
        return addr;
    }
    int Standby::close_s() noexcept
    {
        close_m();
        if (svScok > 0)
        {
            close(svScok);
            svScok = 0;
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
