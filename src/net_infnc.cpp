#include <infnc.h>
#include <unordered_map>

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

    infnc::infnc()
    {
        netcpp_start();
    }

    infnc::~infnc()
    {
        netcpp_stop();
    }
    infnc net;

} // namespace unet
