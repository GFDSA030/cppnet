#include <server.h>
#include <infnc.h>
#include <thread>
namespace unet
{
    server_base::server_base() noexcept {

    };
    server_base::~server_base()
    {
#ifdef NETCPP_SSL_AVAILABLE
        if (ctx != nullptr)
        {
            SSL_CTX_free(ctx);
            ctx = nullptr;
        }
#endif
        close(sock);
        netcpp_stop();
    };
    int server_base::stop() noexcept
    {
        cont = 0;
        return success;
    }
    void server_base::fn2core(server_base *where, void (*fnc_)(net_core &), int socket, const struct sockaddr_in cli, sock_type type_, SSL *ssl_) noexcept
    {
        *where->connections += 1;
        net_core mt(socket, cli, type_, ssl_);
        fnc_(mt);
        mt.close_s();
        *where->connections -= 1;
        return;
    }
    void server_base::run_fn(server_base *where, void (*fnc_)(net_core &), int socket, const struct sockaddr_in cli, sock_type type_, SSL *ssl_, bool thread_) noexcept
    {
        where->connection_no++;
        if (thread_)
        {
            std::thread(fn2core, where, fnc_, socket, cli, type_, ssl_).detach();
        }
        else
        {
            fn2core(where, fnc_, socket, cli, type_, ssl_);
        }
    }
    size_t server_base::get_connection_len() const noexcept
    {
        return *connections;
    }
    size_t server_base::get_connection_no() const noexcept
    {
        return connection_no;
    }
}