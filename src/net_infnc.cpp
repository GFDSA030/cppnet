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