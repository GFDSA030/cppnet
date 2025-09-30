#include <base.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
    net_core::net_core(int socket, const IPaddress cli, sock_type type_, SSL *ssl_) noexcept
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

    IPaddress net_core::remote() const noexcept
    {
        return addr;
    }
} // namespace unet
