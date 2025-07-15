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
        int send_m(const struct sockaddr_in *addr, const char *buf, int len) const noexcept;
        int recv_m(const struct sockaddr_in *addr, char *buf, int len) const noexcept;
        int Tsock = 0;
        int Tport = 0;
        int Rsock = 0;
        int Rport = 0;

    public:
        udp_core();
        ~udp_core();
    };

    class UDP : public udp_core
    {
    public:
        UDP();
        UDP(int Tx_, int Rx_);
        ~UDP();

        int set_port(int Tx_, int Rx_);
        int send_data(const char *addr_, const char *buf, int len);
        int recv_data(char *buf, int len);
        int recv_data(struct sockaddr_in *addr, char *buf, int len);
    };
}

#endif // UDP_H