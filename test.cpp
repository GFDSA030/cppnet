#include <iostream>
#include "include/infnc.h"
#include <unet.h>
int main()
{
    unet::netcpp_start();
    addrinfo hints = {};
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // For wildcard IP address
    addrinfo *res;
    int status = getaddrinfo("qiita.com", NULL, &hints, &res);
    if (status != 0)
    {
        // std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return 1;
    }
    for (addrinfo *p = res; p != nullptr; p = p->ai_next)
    {
        char ipstr[INET_ADDRSTRLEN];
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
        std::cout << "IP Address: " << ipstr << std::endl;
    }
    unet::netcpp_stop();
    return 0;
}