#include <client.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
    ClientTCP::ClientTCP() noexcept
    {
    }

    ClientTCP::ClientTCP(const char *addr_, const int port_) noexcept
    {
        set_type(TCP_c);
        getipaddr(addr_, addr);
        type = TCP_c;
        addr.sin_port = htons(port_);
        addr.sin_family = AF_INET;

        sock = socket(AF_INET, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        this_status = online;
    }

    int ClientTCP::connect_s(const char *addr_, const int port_) noexcept
    {
        close_m();
        set_type(TCP_c);
        getipaddr(addr_, addr);
        type = TCP_c;
        addr.sin_port = htons(port_);
        addr.sin_family = AF_INET;

        sock = socket(AF_INET, SOCK_STREAM, 0);
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        this_status = online;
        connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        return success;
    }

    ClientTCP::~ClientTCP()
    {
        close_m();
    }
}
