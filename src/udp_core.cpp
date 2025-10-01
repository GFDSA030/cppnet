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
    udp_core::udp_core(int Tx_, int Rx_)
    {
        netcpp_start();
        if (Tx_ < 0 || Tx_ > 65535 || Rx_ < 0 || Rx_ > 65535)
        {
            return; //
        }
        Tport = Tx_;
        Rport = Rx_;
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
        IPaddress addr_ = {};
        ((struct sockaddr_in6 *)&addr_)->sin6_family = AF_INET6;
        ((struct sockaddr_in6 *)&addr_)->sin6_port = htons(Rport);
        ((struct sockaddr_in6 *)&addr_)->sin6_addr = in6addr_any;
        bind(Rsock, (struct sockaddr *)&addr_, sizeof(addr_));

#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(Rsock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        udp_no++;
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
    int udp_core::set_port(int Tx_, int Rx_)
    {
        if (Tx_ < 0 || Tx_ > 65535 || Rx_ < 0 || Rx_ > 65535)
        {
            return -1; //
        }
        Tport = Tx_;
        Rport = Rx_;
        IPaddress addr_ = {};
        ((struct sockaddr_in6 *)&addr_)->sin6_family = AF_INET6;
        ((struct sockaddr_in6 *)&addr_)->sin6_port = htons(Rport);
        ((struct sockaddr_in6 *)&addr_)->sin6_addr = in6addr_any;
        bind(Rsock, (struct sockaddr *)&addr_, sizeof(addr_));
        return success;
    }

    int udp_core::send_data(const char *addr, const char *buf, int len)
    {
        IPaddress addr_in = {};
        getipaddrinfo(addr, Tport, addr_in);
        return send_m(&addr_in, buf, len);
    }

    int udp_core::recv_data(IPaddress *addr, char *buf, int len)
    {
        return recv_m(addr, buf, len);
    }
    int udp_core::recv_data(char *buf, int len)
    {
        IPaddress addr;
        return recv_m(&addr, buf, len);
    }
}