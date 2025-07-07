#ifndef SERVER
#define SERVER

#include <base.h>

namespace unet
{
    class Server_com
    {
    private:
        int sock = 0;
        struct sockaddr_in addr = {};
        void (*fnc)(net_core &);
        bool cont = 1;
        sock_type type = TCP_c;

        SSL_CTX *ctx = nullptr;

        bool thread_use = true;

    public:
        Server_com(int port_, void (*fnc_)(net_core &), sock_type type_ = TCP_c, const char *crt = "", const char *pem = "", bool thread_ = true) noexcept;
        ~Server_com();
        sock_type change_type(const sock_type type_) noexcept;
        int listen_p() noexcept;
        int stop() noexcept;
    };

#ifdef SSL_AVAILABLE
    class ServerSSL
    {
    private:
        int sock = 0;
        struct sockaddr_in addr = {};
        void (*fnc)(net_core &);
        bool cont = 1;

        SSL_CTX *ctx = nullptr;

        bool thread_use = true;

    public:
        ServerSSL(int port_, void (*fnc_)(net_core &), const char *crt, const char *pem, bool thread_ = true) noexcept;
        ServerSSL(int port_, void (*fnc_)(net_core &), [[maybe_unused]] sock_type type_ = SSL_c, const char *crt = "", const char *pem = "", bool thread_ = true) noexcept;
        ~ServerSSL();
        int listen_p() noexcept;
        int stop() noexcept;
    };
#endif
    class ServerTCP
    {
    private:
        int sock = 0;
        struct sockaddr_in addr = {};
        void (*fnc)(net_core &);
        bool cont = 1;

        bool thread_use = true;

    public:
        ServerTCP(int port_, void (*fnc_)(net_core &), bool thread_ = true) noexcept;
        ServerTCP(int port_, void (*fnc_)(net_core &), [[maybe_unused]] sock_type type_ = TCP_c, [[maybe_unused]] const char *crt = "", [[maybe_unused]] const char *pem = "", bool thread_ = true) noexcept;
        ~ServerTCP();
        int listen_p() noexcept;
        int stop() noexcept;
    };
}
#endif
