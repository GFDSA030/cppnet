#include <server.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
    ServerTCP::ServerTCP(int port_, void (*fnc_)(net_core &), bool thread_) noexcept
    {
        fnc = fnc_;
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
    }

    ServerTCP::ServerTCP(int port_, void (*fnc_)(net_core &), sock_type type_, const char *crt, const char *pem, bool thread_) noexcept
    {
        fnc = fnc_;
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
    }

    ServerTCP::~ServerTCP()
    {
        close(sock);
    }

    int ServerTCP::listen_p() noexcept
    {
        struct sockaddr_in client;
        uint len;
        int sockcli;
        while (cont == 1)
        {
            len = sizeof(client);
            sockcli = accept(sock, (struct sockaddr *)&client, &len);
#ifndef BLOCKING
            u_long val = 1;
            ioctl(sock, FIONBIO, &val);
#endif // BLOCKING

            run_fn(fnc, sockcli, client, TCP_c, nullptr, thread_use);
        }
        return success;
    }
    int ServerTCP::stop() noexcept
    {
        cont = 0;
        return success;
    }
} // namespace unet
