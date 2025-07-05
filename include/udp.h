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
        int port = 0;

    public:
        udp_core();
        ~udp_core();
    };

    class UDP : public udp_core
    {
    public:
        UDP();
        UDP(int port);
        ~UDP();

        int set_port(int port);
        int send_data(const char *addr_, const char *buf, int len);
        int recv_data(char *buf, int len);
        int recv_data(struct sockaddr_in *addr, char *buf, int len);
        void close_s();
    };
}

#endif // UDP_H