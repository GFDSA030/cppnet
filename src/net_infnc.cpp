#include <infnc.h>
#include <unordered_map>

namespace unet
{
#ifdef NETCPP_SSL_AVAILABLE

#if defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER)

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

#else // not Windows

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

#endif // windows check

#else // NETCPP_SSL_AVAILABLE

#if defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER)

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

#else // not Windows

    void netinit() noexcept
    {
        netcpp_setstatus(online);
    }
    void netquit() noexcept
    {
        netcpp_setstatus(offline);
    }

#endif // windows check

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
