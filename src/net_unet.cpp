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
    int getipaddr(const char *addr_, struct sockaddr_in &ret) noexcept
    {
        struct hostent *hs;
        hs = gethostbyname(addr_);
        if (hs->h_addr_list)
        {
            ret.sin_addr.s_addr = *(u_long *)hs->h_addr_list[0];
        }
        else
        {
            return error;
        }
        return success;
    }

    int getipaddrinfo(const char *addr_, int port_, struct sockaddr_in &ret, sock_type type_) noexcept
    { // TODO:
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family = PF_UNSPEC;
        int err = 0;
        // if ((err = getaddrinfo(addr_, type_ == TCP_c ? "http" : "https", &hints, &res)) != 0)
        if ((err = getaddrinfo(addr_, NULL, &hints, &res)) != 0)
        {
            printf("error %d\n", err);
            return error;
        }
        ret = *(struct sockaddr_in *)res;
        freeaddrinfo(res);
        freeaddrinfo(&hints);
        return success;
    }

}
