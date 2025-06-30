#include <infnc.h>
#include <unordered_map>

namespace unet
{
    void fn2core(void (*fnc_)(net_core &), int socket, const struct sockaddr_in cli, sock_type type_, SSL *ssl_) noexcept
    {
        net_core mt(socket, cli, type_, ssl_);
        fnc_(mt);
        mt.close_s();
        return;
    }
    void run_fn(void (*fnc_)(net_core &), int socket, const struct sockaddr_in cli, sock_type type_, SSL *ssl_, bool thread_) noexcept
    {
        if (thread_)
        {
            std::thread(fn2core, fnc_, socket, cli, type_, ssl_).detach();
        }
        else
        {
            fn2core(fnc_, socket, cli, type_, ssl_);
        }
    }

    int startGC()
    {
        return error;
    }

    int setthread(std::thread *p)
    {
        return error;
    }

    int freethread(std::thread *p)
    {
        return error;
    }
} // namespace unet