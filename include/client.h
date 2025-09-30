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
}
#endif
