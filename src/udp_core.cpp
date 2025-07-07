#include <udp.h>

namespace unet
{
    size_t udp_core::udp_no = 0;

    int udp_core::send_m(struct sockaddr_in *addr, const char *buf, int len)
    {
        return sendto(sock, buf, len, 0, (struct sockaddr *)addr, sizeof(struct sockaddr_in));
    }
    int udp_core::recv_m(struct sockaddr_in *addr, char *buf, int len)
    {
        socklen_t addr_len = sizeof(struct sockaddr_in);
        return recvfrom(sock, buf, len, 0, (struct sockaddr *)addr, &addr_len);
    }
    udp_core::udp_core()
    {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }
#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(sock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        udp_no++;
    }

    udp_core::~udp_core()
    {
        if (sock > 0)
        {
            close(sock);
        }
        udp_no--;
    }
}