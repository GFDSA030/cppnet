#include <udp.h>
#include <infnc.h>

namespace unet
{
    UDP::UDP()
    {
    }
    UDP::UDP(int port_)
    {
        set_port(port_);
    }

    UDP::~UDP()
    {
    }
    int UDP::set_port(int port_)
    {
        if (port_ < 0 || port_ > 65535)
        {
            return -1; //
        }
        port = port_;
    }

    int UDP::send_data(const char *addr, const char *buf, int len)
    {
        struct sockaddr_in addr_in;
        getipaddr(addr, addr_in);
        addr_in.sin_port = htons(port);
        addr_in.sin_family = AF_INET;
        return send_m(&addr_in, buf, len);
    }

    int UDP::recv_data(struct sockaddr_in *addr, char *buf, int len)
    {
        socklen_t addr_len = sizeof(*addr);
        return recv_m(addr, buf, len);
    }
    int UDP::recv_data(char *buf, int len)
    {
        return recv(sock, buf, len, 0);
    }
} // namespace unet