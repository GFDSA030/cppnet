#include <base.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
    net_coreIPV6::net_coreIPV6(int socket, const addrinfo cli, sock_type type_, SSL *ssl_) noexcept
    {
        sock = socket;
        addrV6 = cli;
        type = type_;
        set_type(type_);
        this_status = online;
        ssl = ssl_;
    }

    net_coreIPV6::~net_coreIPV6()
    {
        close_m();
    }

    addrinfo net_coreIPV6::remote() const noexcept
    {
        return addrV6;
    }
} // namespace unet
