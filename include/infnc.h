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
    void run_fn(void (*fnc_)(net_core &), int socket, const struct sockaddr_in cli, sock_type type_, SSL *ssl_, bool thread_) noexcept;
    int startGC();
    int setthread(std::thread *p);
    int freethread(std::thread *p);
} // namespace unet

#endif