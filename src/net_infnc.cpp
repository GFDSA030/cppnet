#include <infnc.h>
#include <unordered_map>

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