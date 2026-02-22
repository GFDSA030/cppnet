#include <server.h>
#include <infnc.h>
#include <errno.h>
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
            close(sock);
            sock = -1;
            return;
        }
        const int opt = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
        {
            perror("setsockopt SO_REUSEADDR error");
            close(sock);
            sock = -1;
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
            close(sock);
            sock = -1;
            return;
        }
        if (listen(sock, 25) < 0)
        {
            perror("Error. Cannot listen socket");
            close(sock);
            sock = -1;
            return;
        }
        // Keep accept loop interruptible so stop() can terminate promptly.
        u_long listen_nonblock = 1;
        if (ioctl(sock, FIONBIO, &listen_nonblock) < 0)
        {
            perror("ioctl(FIONBIO) failed on listen socket");
            close(sock);
            sock = -1;
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
                SSL_CTX_free(ctx);
                ctx = nullptr;
                close(sock);
                sock = -1;
                return;
            }
            // load crt
            if (!SSL_CTX_use_certificate_file(ctx, crt, SSL_FILETYPE_PEM))
            {
                perror("Error: SSL_CTX_use_certificate_file()\n");
                ERR_print_errors_fp(stderr);
                SSL_CTX_free(ctx);
                ctx = nullptr;
                close(sock);
                sock = -1;
                return;
            }
            // load private key
            if (!SSL_CTX_use_PrivateKey_file(ctx, pem, SSL_FILETYPE_PEM))
            {
                perror("Error: SSL_CTX_use_PrivateKey_file()\n");
                ERR_print_errors_fp(stderr);
                SSL_CTX_free(ctx);
                ctx = nullptr;
                close(sock);
                sock = -1;
                return;
            }
#else
        fprintf(stderr, "ssl isn't avilable\n");
#endif
        }
    }

    Server::~Server()
    {
        stop();
    }

    int Server::listen_m() noexcept
    {
        if (sock < 0)
            return error;

        IPaddress client;
        int sockcli;
        while (cont.load())
        {
            cleanup_worker_threads();
            fd_set readfds;
            FD_ZERO(&readfds);
#if defined(_WIN32) || defined(__MINGW32__)
            SOCKET s = (SOCKET)sock;
#else
            int s = sock;
#endif
            FD_SET(s, &readfds);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 20 * 1000;

            int sel = select((int)(s + 1), &readfds, NULL, NULL, &tv);
            if (sel == 0)
            {
                // timeout: re-check cont and continue
                continue;
            }
            if (sel < 0)
            {
                if (!cont.load() || sock < 0)
                    break;
                perror("select() failed");
                continue;
            }

            socklen_t len = sizeof(client);
#ifdef __MINGW32__
            sockcli = accept(sock, (struct sockaddr *)&client, (int *)&len);
#else
            sockcli = accept(sock, (struct sockaddr *)&client, &len);
#endif
            if (sockcli < 0)
            {
                if (!cont.load() || sock < 0)
                    break;
#if defined(_WIN32) || defined(__MINGW32__)
                const int wsaerr = WSAGetLastError();
                if (wsaerr == WSAEWOULDBLOCK)
                    continue;
#else
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    continue;
#if defined(__linux__) || defined(__APPLE__)
                if (errno == EBADF || errno == ENOTSOCK || errno == EINVAL)
                    break;
#endif
#endif
                perror("accept() failed");
                continue;
            }
#ifdef NETCPP_BLOCKING
            u_long val = 0;
            ioctl(sockcli, FIONBIO, &val);
#else
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
        if (sock < 0)
        {
            return error;
        }

        if (block)
            return listen_m();
        else
        {
            if (listen_thread.joinable())
                return error;
            cont = true;
            listen_thread = std::thread(&Server::listen_m, this);
            return success;
        }
    }
}
