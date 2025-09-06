#ifndef SERVER
#define SERVER

#include <base.h>
#include <memory>
namespace unet
{
    typedef void (*svrCallbackFn)(net_core &, void *);
    class server_base
    {
    private:
        static void fn2core(server_base *where, svrCallbackFn fnc_, int socket, const struct sockaddr_in cli, sock_type type_, SSL *ssl_, void *Udata) noexcept;
        std::shared_ptr<size_t> connections = std::make_shared<size_t>(0);
        size_t connection_no = 0;
        bool cont = 1;
        void *UserData = nullptr;

    protected:
        server_base() noexcept;
        ~server_base();

        static void run_fn(server_base *where, svrCallbackFn fnc_, int socket, const struct sockaddr_in cli, sock_type type_, SSL *ssl_, bool thread_, void *Udata) noexcept;

        int sock = 0;
        struct sockaddr_in addr = {};
        svrCallbackFn fnc = nullptr;
        sock_type type = TCP_c;
        SSL_CTX *ctx = nullptr;
        bool thread_use = true;


    public:
        int setUserData(void *data) noexcept;
        size_t get_connection_len() const noexcept;
        size_t get_connection_no() const noexcept;

        int stop() noexcept;
    };

    class Server : public server_base
    {
    private:
        int listen_m() noexcept;

    public:
        Server(int port_, svrCallbackFn fnc_, sock_type type_ = TCP_c, const char *crt = "", const char *pem = "", bool thread_ = true) noexcept;
        ~Server();
        sock_type change_type(const sock_type type_) noexcept;
        int listen_p(bool block = true) noexcept;
    };
    typedef Server Server_com;

#ifdef NETCPP_SSL_AVAILABLE
    class ServerSSL : public server_base
    {
    private:
        int listen_m() noexcept;

    public:
        ServerSSL(int port_, svrCallbackFn fnc_, const char *crt, const char *pem, bool thread_ = true) noexcept;
        ServerSSL(int port_, svrCallbackFn fnc_, [[maybe_unused]] sock_type type_ = SSL_c, const char *crt = "", const char *pem = "", bool thread_ = true) noexcept;
        ~ServerSSL();
        int listen_p(bool block = true) noexcept;
    };
#endif
    class ServerTCP : public server_base
    {
    private:
        int listen_m() noexcept;

    public:
        ServerTCP(int port_, svrCallbackFn fnc_, bool thread_ = true) noexcept;
        ServerTCP(int port_, svrCallbackFn fnc_, [[maybe_unused]] sock_type type_ = TCP_c, [[maybe_unused]] const char *crt = "", [[maybe_unused]] const char *pem = "", bool thread_ = true) noexcept;
        ~ServerTCP();
        int listen_p(bool block = true) noexcept;
    };
}
#endif
