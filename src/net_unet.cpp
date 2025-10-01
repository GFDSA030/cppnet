#include <base.h>
#include <infnc.h>
#include <string>
#include <thread>

namespace unet
{

    status netcpp_status = offline;
    size_t netcpp_using_no = 0;
    int netcpp_start() noexcept
    {
        netcpp_using_no++;
        if (netcpp_status == offline)
        {
            netinit();
            netcpp_status = online;
            return success;
        }
        return success;
    }
    int netcpp_stop() noexcept
    {
        netcpp_using_no--;
        if ((netcpp_status == online) && (netcpp_using_no == 0))
        {
            netquit();
            netcpp_status = offline;
            return success;
        }
        return success;
    }
    int netcpp_setstatus(status s) noexcept
    {
        if (s == online)
        {
            netcpp_status = s;
            netcpp_using_no++;
            return success;
        }
        if (s == offline)
        {
            netcpp_status = s;
            netcpp_using_no--;
            return success;
        }
        return error;
    }
    int getipaddr(const char *addr_, IPaddress &ret) noexcept
    {
        struct hostent *hs;
        hs = gethostbyname(addr_);
        if (hs->h_addr_list)
        {
            ((struct sockaddr_in *)&ret)->sin_family = AF_INET;
            ((struct sockaddr_in *)&ret)->sin_port = 0;
            ((struct sockaddr_in *)&ret)->sin_addr.s_addr = *(u_long *)hs->h_addr_list[0];
            // ret.sin_addr.s_addr = *(u_long *)hs->h_addr_list[0];
        }
        else
        {
            return error;
        }
        return success;
    }

    int getipaddrinfo(const char *addr_, int port_, IPaddress &ret, sock_type type_) noexcept
    {
        addrinfo hints = {}, *res;
        hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
        hints.ai_socktype = type_ == sock_type::TCP_c ? SOCK_STREAM : (type_ == sock_type::UDP_c ? SOCK_DGRAM : NULL);
        hints.ai_flags = AI_PASSIVE; // For wildcard IP address
        int err = 0;
        // if ((err = getaddrinfo(addr_, type_ == TCP_c ? "http" : "https", &hints, &res)) != 0)
        if ((err = getaddrinfo(addr_, NULL, &hints, &res)) != 0)
        {
            printf("error %d\n", err);
            return error;
        }
        if (res == nullptr)
        {
            return error;
        }
        // memcpy(&ret, res->ai_addr, sizeof(addrinfo));
        memcpy(&ret, res->ai_addr, sizeof(IPaddress));
        freeaddrinfo(res);
        ret.ss_family = res->ai_family;
        if (ret.ss_family == AF_INET)
            ((struct sockaddr_in *)&ret)->sin_port = htons(port_);
        if (ret.ss_family == AF_INET6)
            ((struct sockaddr_in6 *)&ret)->sin6_port = htons(port_);
        return success;
    }
    std::string ip2str(const IPaddress &addr) noexcept
    {
        char ipstr[INET6_ADDRSTRLEN] = {};
        void *addr_ptr;
        // if (addr.ai_family == AF_INET) // IPv4
        if (addr.ss_family == AF_INET) // IPv4
        {
            addr_ptr = &(((struct sockaddr_in *)&addr)->sin_addr);
        }
        else // IPv6
        {
            addr_ptr = &(((struct sockaddr_in6 *)&addr)->sin6_addr);
        }
        // inet_ntop(addr.ai_family, addr_ptr, ipstr, sizeof(ipstr));
        inet_ntop(((struct sockaddr_in *)&addr)->sin_family, addr_ptr, ipstr, sizeof(ipstr));
        return std::string(ipstr);
    }
}
