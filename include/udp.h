#ifndef UDP_H
#define UDP_H
#include <netdefs.h>

namespace unet
{
    class udp_core // TODO: 構成考える
    {
    private:
        static size_t udp_no;

    protected:
        int send_m(const IPaddress *addr, const char *buf, int len) const noexcept;
        int recv_m(IPaddress *addr, char *buf, int len) const noexcept;
        mutable int Tsock = 0;
        int Tport = 0;
        mutable int Rsock = 0;
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
        int recv_data(IPaddress *addr, char *buf, int len);
    };
}

#endif // UDP_H