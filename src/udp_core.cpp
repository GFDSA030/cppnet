#include <udp.h>
#include <infnc.h>
namespace unet
{
    // TODO: 汚いコードをなんとかする
    size_t udp_core::udp_no = 0;

    int udp_core::send_m(const IPaddress *addr, const char *buf, int len) const noexcept
    {
        Tsock = socket(addr->ss_family, SOCK_DGRAM, 0);
        if (Tsock < 0)
        {
            perror("udp send socket()");
            return error;
        }
        socklen_t addrlen = 0;
        if (addr->ss_family == AF_INET)
            addrlen = sizeof(struct sockaddr_in);
        else if (addr->ss_family == AF_INET6)
            addrlen = sizeof(struct sockaddr_in6);
        else
            addrlen = sizeof(struct sockaddr_storage);

        int ret = sendto(Tsock, buf, len, 0, (struct sockaddr *)addr, addrlen);
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
            perror("Error. Cannot make Rsock");
            Rsock = 0;
            return;
        }
        int off = 0;
        if (setsockopt(Rsock, IPPROTO_IPV6, IPV6_V6ONLY,
                       (char *)&off, sizeof(off)) < 0)
        {
            perror("setsockopt IPV6_V6ONLY");
        }
        IPaddress addr_ = {0};
        addr_.ss_family = AF_INET6;
        ((struct sockaddr_in6 *)&addr_)->sin6_port = htons(Rport);
        ((struct sockaddr_in6 *)&addr_)->sin6_addr = in6addr_any;
        if (bind(Rsock, (struct sockaddr *)&addr_, sizeof(addr_)) < 0)
        {
            perror("bind Rsock failed");
            close(Rsock);
            Rsock = 0;
            return;
        }

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
            perror("Error. Cannot make Rsock");
            Rsock = 0;
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
            shutdown(Tsock, SHUT_RW);
            close(Tsock);
            Tsock = 0;
        }
        if (Rsock > 0)
        {
            shutdown(Rsock, SHUT_RW);
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
        IPaddress addr_ = {0};
        addr_.ss_family = AF_INET6;
        ((struct sockaddr_in6 *)&addr_)->sin6_port = htons(Rport);
        ((struct sockaddr_in6 *)&addr_)->sin6_addr = in6addr_any;
        if (bind(Rsock, (struct sockaddr *)&addr_, sizeof(addr_)) < 0)
        {
            perror("bind Rsock failed in set_port");
            return error;
        }
        return success;
    }

    int udp_core::send_data(const char *addr, const char *buf, int len)
    {
        IPaddress addr_in = {0};
        getipaddrinfo(addr, Tport, addr_in, UDP_c);
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