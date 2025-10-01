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
        // Copy the first result's sockaddr into ret safely.
        addrinfo *first = res;
        // initialize ret
        memset(&ret, 0, sizeof(IPaddress));
        size_t copylen = 0;
#ifdef _WIN32
        // on Windows, ai_addrlen may not be reliable; choose by family
        if (first->ai_family == AF_INET)
            copylen = sizeof(struct sockaddr_in);
        else if (first->ai_family == AF_INET6)
            copylen = sizeof(struct sockaddr_in6);
        else
            copylen = sizeof(struct sockaddr_storage);
#else
        copylen = first->ai_addrlen;
        if (copylen == 0 || copylen > sizeof(IPaddress))
            copylen = sizeof(IPaddress);
#endif
        if (first->ai_addr != nullptr)
            memcpy(&ret, first->ai_addr, copylen);

        // set family and port based on the addrinfo entry
        ret.ss_family = first->ai_family;
        if (ret.ss_family == AF_INET)
        {
            ((struct sockaddr_in *)&ret)->sin_port = htons(port_);
        }
        else if (ret.ss_family == AF_INET6)
        {
            ((struct sockaddr_in6 *)&ret)->sin6_port = htons(port_);
        }

        freeaddrinfo(res);
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
        // use ss_family stored in sockaddr_storage
        inet_ntop(addr.ss_family, addr_ptr, ipstr, sizeof(ipstr));
        return std::string(ipstr);
    }
}
