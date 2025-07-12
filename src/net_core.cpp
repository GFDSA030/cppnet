#include <base.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
    net_core::net_core(int socket, const struct sockaddr_in cli, sock_type type_, SSL *ssl_) noexcept
    {
        sock = socket;
        addr = cli;
        type = type_;
        set_type(type_);
        this_status = online;
        ssl = ssl_;
    }

    net_core::~net_core()
    {
        close_m();
    }

    struct sockaddr_in net_core::remote() const noexcept
    {
        return addr;
    }
} // namespace unet
