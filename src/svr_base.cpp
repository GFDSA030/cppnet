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
        if (sock > 0)
        {
            close(sock);
            sock = 0;
        }
        netcpp_stop();
    };
    int server_base::stop() noexcept
    {
        cont = 0;
        return success;
    }
    int server_base::setUserData(void *data) noexcept
    {
        UserData = data;
        return success;
    }
    void server_base::fn2core(server_base *where, svrCallbackFn fnc_, int socket, const IPaddress cli, sock_type type_, SSL *ssl_, void *Udata) noexcept
    {
        std::shared_ptr<size_t> connections = where->connections;
        net_core mt(socket, cli, type_, ssl_);
        try
        {
            if (fnc_)
                fnc_(mt, Udata);
        }
        catch (...)
        {
            // swallow exceptions to avoid terminating the thread unexpectedly
        }
        // ensure socket is closed and decrement connection counter if used
        mt.close_s();
        if (where->connection_no > 0)
            where->connection_no--;
        return;
    }
    void server_base::run_fn(server_base *where, svrCallbackFn fnc_, int socket, const IPaddress cli, sock_type type_, SSL *ssl_, bool thread_, void *Udata) noexcept
    {
        where->connection_no++;
        if (thread_)
        {
            std::thread(fn2core, where, fnc_, socket, cli, type_, ssl_, Udata).detach();
        }
        else
        {
            fn2core(where, fnc_, socket, cli, type_, ssl_, Udata);
        }
    }
    size_t server_base::get_connection_len() const noexcept
    {
        return connections.use_count() - 1;
    }
    size_t server_base::get_connection_no() const noexcept
    {
        return connection_no;
    }
}