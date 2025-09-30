#include <client.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
    ClientTCPipV6::ClientTCPipV6() noexcept
    {
    }

    ClientTCPipV6::ClientTCPipV6(const char *addr_, const int port_) noexcept
    {
        set_type(TCP_c);
        getipaddrinfo(addr_, port_, addrV6, TCP_c);
        type = TCP_c;

        sock = socket(addrV6.ai_family, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        connect(sock, addrV6.ai_addr, addrV6.ai_addrlen);
        this_status = online;
    }

    int ClientTCPipV6::connect_s(const char *addr_, const int port_) noexcept
    {
        close_m();
        set_type(TCP_c);
        getipaddrinfo(addr_, port_, addrV6, TCP_c);
        type = TCP_c;

        sock = socket(addrV6.ai_family, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        connect(sock, addrV6.ai_addr, addrV6.ai_addrlen);
        this_status = online;
        return success;
    }

    ClientTCPipV6::~ClientTCPipV6()
    {
        close_m();
    }
}
