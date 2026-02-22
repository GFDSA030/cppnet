#pragma once

#include <base.h>
#include <thread>
#include "cryptaes.h"

#if defined(__MINGW32__) // OS

namespace unet
{
    typedef int uint;
    constexpr int BUF_SIZE = 1024;
}

#elif defined(_MSC_VER) // OS

namespace unet
{
    typedef int (*Fnc)(SOCKET);
    Fnc close = closesocket;
    typedef int uint;
    constexpr int BUF_SIZE = 1024;
}

#elif defined(__linux__) // OS

namespace unet
{
    constexpr int BUF_SIZE = 1024;
}

#elif defined(__APPLE__) // OS

namespace unet
{
    constexpr int BUF_SIZE = 1024;
}

#endif // OS

#define SHUT_RC 0
#define SHUT_SD 1
#define SHUT_RW 2

namespace unet
{
    void netinit() noexcept;
    void netquit() noexcept;
    int netcpp_setstatus(status s) noexcept;
    int Def_connect(int &sock, sock_type &type, status &this_status, int &port, IPaddress &addr, const char *addr_, SSL *&ssl, SSL_CTX *&ctx) noexcept;
    class infnc
    {
    private:
    public:
        infnc();
        ~infnc();
    };

} // namespace unet
