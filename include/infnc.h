#ifndef INFNC
#define INFNC

#include <base.h>
#include <thread>

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

#define SHUT_RW 2

namespace unet
{
    int startGC();
    int netcpp_start() noexcept;
    int netcpp_stop()noexcept;
    int netcpp_setstatus(status s)noexcept;
    int setthread(std::thread *p);
    int freethread(std::thread *p);
} // namespace unet

#endif