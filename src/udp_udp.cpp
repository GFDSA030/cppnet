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
        return success;
    }

    int UDP::send_data(const char *addr, const char *buf, int len)
    {
        IPaddress addr_in;
        getipaddr(addr, addr_in);
        ((struct sockaddr_in *)&addr_in)->sin_addr.s_addr = ((struct sockaddr_in *)&addr_in)->sin_addr.s_addr;
        ((struct sockaddr_in *)&addr_in)->sin_port = htons(Tport);
        ((struct sockaddr_in *)&addr_in)->sin_family = AF_INET;
        // addr_in.sin_port = htons(Tport);
        // addr_in.sin_family = AF_INET;
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