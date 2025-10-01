#include <udp.h>
#include <infnc.h>
#include <iostream>
namespace unet
{
    UDP::UDP()
    {
    }
    UDP::UDP(int Tx_, int Rx_)
    {
        set_port(Tx_, Rx_);
    }

    UDP::~UDP()
    {
    }
    int UDP::set_port(int Tx_, int Rx_)
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

    int UDP::send_data(const char *addr, const char *buf, int len)
    {
        IPaddress addr_in = {};
        getipaddrinfo(addr, Tport, addr_in);
        return send_m(&addr_in, buf, len);
    }

    int UDP::recv_data(IPaddress *addr, char *buf, int len)
    {
        return recv_m(addr, buf, len);
    }
    int UDP::recv_data(char *buf, int len)
    {
        IPaddress addr;
        return recv_m(&addr, buf, len);
    }
} // namespace unet