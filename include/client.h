#ifndef CLIENT
#define CLIENT

#include <base.h>

namespace unet
{

    class Client : public net_base
    {
    private:
    public:
        Client() noexcept;
        Client(const char *addr_, const sock_type type_, const int port_ = -1) noexcept;
        int connect_s(const char *addr_, const sock_type type_, const int port_ = -1) noexcept;
        sock_type change_type(const sock_type type_) noexcept;
        ~Client();
    };
    typedef Client Client_com;

    class ClientTCP : public net_base
    {
    private:
    public:
        ClientTCP() noexcept;
        ClientTCP(const char *addr_, const int port_ = 80) noexcept;
        int connect_s(const char *addr_, const int port_ = 80) noexcept;
        ~ClientTCP();
    };
    class ClientTCPipV6 : public net_base
    {
    private:
        addrinfo addrV6 = {};

    public:
        ClientTCPipV6() noexcept;
        ClientTCPipV6(const char *addr_, const int port_ = 80) noexcept;
        int connect_s(const char *addr_, const int port_ = 80) noexcept;
        ~ClientTCPipV6();
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
    class ClientSSLipV6 : public net_base
    {
    private:
        addrinfo addrV6 = {};

    public:
        ClientSSLipV6() noexcept;
        ClientSSLipV6(const char *addr_, const int port_ = 443) noexcept;
        int connect_s(const char *addr_, const int port_ = 443) noexcept;
        ~ClientSSLipV6();
    };
#endif // NETCPP_SSL_AVAILABLE
}
#endif
