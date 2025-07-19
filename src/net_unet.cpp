#include <base.h>
#include <infnc.h>
#include <string>
#include <thread>

namespace unet
{

#ifdef NETCPP_SSL_AVAILABLE

#ifdef __WIN32

    WSADATA data;
    void netinit() noexcept
    {
        netcpp_setstatus(online);
        WSAStartup(MAKEWORD(2, 2), &data);
        SSL_load_error_strings();
        SSL_library_init();
    }
    void netquit() noexcept
    {
        ERR_free_strings();
        WSACleanup();
        netcpp_setstatus(offline);
    }

#else //__WIN32

    void netinit() noexcept
    {
        netcpp_setstatus(online);
        SSL_load_error_strings();
        SSL_library_init();
    }
    void netquit() noexcept
    {
        ERR_free_strings();
        netcpp_setstatus(offline);
    }

#endif //__WIN32

#else // NETCPP_SSL_AVAILABLE

#ifdef __WIN32

    WSADATA data;
    void netinit() noexcept
    {
        netcpp_setstatus(online);
        WSAStartup(MAKEWORD(2, 0), &data);
    }
    void netquit() noexcept
    {
        WSACleanup();
        netcpp_setstatus(offline);
    }

#else //__WIN32

    void netinit() noexcept
    {
        netcpp_setstatus(online);
    }
    void netquit() noexcept
    {
        netcpp_setstatus(offline);
    }

#endif //__WIN32

#endif // NETCPP_SSL_AVAILABLE

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
        if ((err = getaddrinfo(addr_, type_ == TCP_c ? "http" : "https", &hints, &res)) != 0)
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
