#include <udp.h>
#include <infnc.h>
namespace unet
{
    size_t udp_core::udp_no = 0;

    int udp_core::send_m(const IPaddress *addr, const char *buf, int len) const noexcept
    {
        Tsock = socket(addr->ss_family, SOCK_DGRAM, 0);
        int ret = sendto(Tsock, buf, len, 0, (struct sockaddr *)addr, sizeof(*addr));
        close(Tsock);
        Tsock = 0;
        return ret;
    }
    int udp_core::recv_m(IPaddress *addr, char *buf, int len) const noexcept
    {
        socklen_t addr_len = sizeof(IPaddress);
        int ret = recvfrom(Rsock, buf, len, 0, (struct sockaddr *)addr, &addr_len);
        return ret;
    }
    udp_core::udp_core()
    {
        netcpp_start();
        Rsock = socket(AF_INET6, SOCK_DGRAM, 0);
        if (Rsock < 0)
        {
            fprintf(stderr, "Error. Cannot make socket\n");
            return;
        }
        int off = 0;
        if (setsockopt(Rsock, IPPROTO_IPV6, IPV6_V6ONLY,
                       (char *)&off, sizeof(off)) < 0)
        {
            perror("setsockopt IPV6_V6ONLY");
        }

#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(Rsock, FIONBIO, &val);
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
            Rsock = 0;
        }
        udp_no--;
        netcpp_stop();
    }
}