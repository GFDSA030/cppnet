#include <unet.h>

int main()
{
    unet::UDP udp(12345, 12346);
    udp.send_data("127.0.0.1", "Hello, UDP!", 11);
    char buf[512];
    unet::IPaddress addr;
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int r = udp.recv_data(&addr, buf, sizeof(buf) - 1);
        // if (r > 0)
        {
            printf("Received from %s: %s\n", unet::ip2str(addr).c_str(), buf);
            udp.send_data(unet::ip2str(addr).c_str(), buf, r);
        }
    }
    return 0;
}