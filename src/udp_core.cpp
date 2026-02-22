#include <udp.h>
#include <infnc.h>
namespace unet
{
    // TODO: 汚いコードをなんとかする
    size_t udp_core::udp_no = 0;

    int udp_core::send_m(const IPaddress *addr, const char *buf, int len) const noexcept
    {
        TXsock = socket(addr->ss_family, SOCK_DGRAM, 0);
        if (TXsock < 0)
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

        int ret = sendto(TXsock, buf, len, 0, (struct sockaddr *)addr, addrlen);
        close(TXsock);
        TXsock = 0;
        return ret;
    }
    int udp_core::recv_m(IPaddress *addr, char *buf, int len, int32_t timeout) const noexcept
    {
        if (RXsock <= 0)
            return error;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(RXsock, &readfds);

        struct timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        int sel = select(RXsock + 1, &readfds, NULL, NULL, timeout >= 0 ? &tv : NULL);
        if (sel <= 0)
        {
            if (sel == 0)
                return 0;
            return error;
        }

        socklen_t addr_len = sizeof(IPaddress);
        int ret = recvfrom(RXsock, buf, len, 0, (struct sockaddr *)addr, &addr_len);
        return ret;
    }
    udp_core::udp_core(int Tx_, int Rx_)
    {
        netcpp_start();
        if (Tx_ < 0 || Tx_ > 65535 || Rx_ < 0 || Rx_ > 65535)
        {
            return; //
        }
        TXport = Tx_;
        RXport = Rx_;
        RXsock = socket(AF_INET6, SOCK_DGRAM, 0);
        if (RXsock < 0)
        {
            perror("Error. Cannot make RXsock");
            RXsock = 0;
            return;
        }
        int off = 0;
        if (setsockopt(RXsock, IPPROTO_IPV6, IPV6_V6ONLY,
                       (char *)&off, sizeof(off)) < 0)
        {
            perror("setsockopt IPV6_V6ONLY");
        }
        IPaddress addr_ = {0};
        addr_.ss_family = AF_INET6;
        ((struct sockaddr_in6 *)&addr_)->sin6_port = htons(RXport);
        ((struct sockaddr_in6 *)&addr_)->sin6_addr = in6addr_any;
        if (bind(RXsock, (struct sockaddr *)&addr_, sizeof(addr_)) < 0)
        {
            perror("bind RXsock failed");
            close(RXsock);
            RXsock = 0;
            return;
        }

#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(RXsock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        udp_no++;
    }
    udp_core::udp_core()
    {
        netcpp_start();
        RXsock = socket(AF_INET6, SOCK_DGRAM, 0);
        if (RXsock < 0)
        {
            perror("Error. Cannot make RXsock");
            RXsock = 0;
            return;
        }
        int off = 0;
        if (setsockopt(RXsock, IPPROTO_IPV6, IPV6_V6ONLY,
                       (char *)&off, sizeof(off)) < 0)
        {
            perror("setsockopt IPV6_V6ONLY");
        }

#ifndef NETCPP_BLOCKING
        u_long val = 1;
        ioctl(RXsock, FIONBIO, &val);
#endif // NETCPP_BLOCKING
        udp_no++;
    }

    udp_core::~udp_core()
    {
        if (TXsock > 0)
        {
            shutdown(TXsock, SHUT_RW);
            close(TXsock);
            TXsock = 0;
        }
        if (RXsock > 0)
        {
            shutdown(RXsock, SHUT_RW);
            close(RXsock);
            RXsock = 0;
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
        TXport = Tx_;
        RXport = Rx_;
        IPaddress addr_ = {0};
        addr_.ss_family = AF_INET6;
        ((struct sockaddr_in6 *)&addr_)->sin6_port = htons(RXport);
        ((struct sockaddr_in6 *)&addr_)->sin6_addr = in6addr_any;
        if (bind(RXsock, (struct sockaddr *)&addr_, sizeof(addr_)) < 0)
        {
            perror("bind RXsock failed in set_port");
            return error;
        }
        return success;
    }

    int udp_core::send_data(const char *addr, const char *buf, int len)
    {
        IPaddress addr_in = {0};
        getipaddrinfo(addr, TXport, addr_in, UDP_c);
        return send_m(&addr_in, buf, len);
    }

    int udp_core::recv_data(IPaddress *addr, char *buf, int len, int32_t timeout)
    {
        return recv_m(addr, buf, len, timeout);
    }
    int udp_core::recv_data(char *buf, int len, int32_t timeout)
    {
        IPaddress addr;
        return recv_m(&addr, buf, len, timeout);
    }
}
