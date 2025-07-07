#ifndef CLIENT
#define CLIENT

#include <base.h>

namespace unet
{

    class Client_com : public net_base
    {
    private:
    public:
        Client_com() noexcept;
        Client_com(const char *addr_, const sock_type type_, const int port_ = -1) noexcept;
        int connect_s(const char *addr_, const sock_type type_, const int port_ = -1) noexcept;
        sock_type change_type(const sock_type type_) noexcept;
        ~Client_com();
    };

    class ClientTCP : public net_base
    {
    private:
    public:
        ClientTCP() noexcept;
        ClientTCP(const char *addr_, const int port_ = 80) noexcept;
        int connect_s(const char *addr_, const int port_ = 80) noexcept;
        ~ClientTCP();
    };
#ifdef NETCPP_SSL_AVAILABLE
    class ClientSSL : public net_base
    {
    private:
    public:
        ClientSSL() noexcept;
        ClientSSL(const char *addr_, const int port_ = 443) noexcept;
        int connect_s(const char *addr_, const int port_ = 443) noexcept;
        ~ClientSSL();
    };
#endif // NETCPP_SSL_AVAILABLE
}
#endif
