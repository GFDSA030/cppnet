#ifndef UDP_H
#define UDP_H
#include <netdefs.h>

namespace unet
{
    class udp_core
    {
    private:
        static size_t udp_no;

    protected:
        int send_m(struct sockaddr_in *addr, const char *buf, int len);
        int recv_m(struct sockaddr_in *addr, char *buf, int len);
        int sock = 0;

    public:
        udp_core();
        ~udp_core();
    };

}

#endif // UDP_H