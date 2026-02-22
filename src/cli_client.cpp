#include <client.h>
#include <infnc.h>
#include <string>
#include <thread>

namespace unet
{
    Client::Client(const char *addr_, const sock_type type_, const int port_) noexcept
    {
        set_type(type_);
        type = type_;
        int port = (port_ == -1) ? (int)type_ : port_;
        if (Def_connect(sock, type, this_status, port, addr, addr_, ssl, ctx) != success)
        {
            this_status = offline;
            return;
        }
        this_status = online;
    }

    int Client::connect_s(const char *addr_, const sock_type type_, const int port_) noexcept
    {
        close_m();
        set_type(type_);
        type = type_;
        int port = (port_ == -1) ? (int)type_ : port_;
        if (Def_connect(sock, type, this_status, port, addr, addr_, ssl, ctx) != success)
        {
            this_status = offline;
            return error;
        }
        this_status = online;
        return success;
    }

    Client::Client() noexcept
    {
    }

    Client::~Client()
    {
        close_m();
    }

}
