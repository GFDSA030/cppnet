#include <udp.h>
#include <infnc.h>
namespace unet
{
    size_t udp_core::udp_no = 0;

    int udp_core::send_m(const IPaddress *addr, const char *buf, int len) const noexcept
    {
        return sendto(Tsock, buf, len, 0, (struct sockaddr *)addr, sizeof(*addr));
    }
    int udp_core::recv_m(const IPaddress *addr, char *buf, int len) const noexcept
    {
        socklen_t addr_len = sizeof(*addr);
        return recvfrom(Rsock, buf, len, 0, (struct sockaddr *)addr, &addr_len);
    }
    udp_core::udp_core()
    {
        netcpp_start();
        Tsock = socket(AF_INET, SOCK_DGRAM, 0);
        Rsock = socket(AF_INET, SOCK_DGRAM, 0);
        if ((Tsock | Rsock) < 0)
        {
            perror("socket");
            return;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(Rport);
        addr.sin_addr.s_addr = INADDR_ANY;
        bind(Rsock, (struct sockaddr *)&addr, sizeof(addr));

#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        udp_no++;
    }

    udp_core::~udp_core()
    {
        if (Tsock > 0)
        {
            close(Tsock);
        }
        if (Rsock > 0)
        {
            close(Rsock);
        }
        udp_no--;
        netcpp_stop();
    }
}