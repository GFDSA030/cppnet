#include <server.h>
#include <infnc.h>
#include <thread>
namespace unet
{
    server_baseIPV6::server_baseIPV6() noexcept {
    };
    server_baseIPV6::~server_baseIPV6()
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
    int server_baseIPV6::stop() noexcept
    {
        cont = 0;
        return success;
    }
    int server_baseIPV6::setUserData(void *data) noexcept
    {
        UserData = data;
        return success;
    }
    void server_baseIPV6::fn2core(server_baseIPV6 *where, svrCallbackFnIPV6 fnc_, int socket, const addrinfo cli, sock_type type_, SSL *ssl_, void *Udata) noexcept
    {
        std::shared_ptr<size_t> connections = where->connections;
        net_coreIPV6 mt(socket, cli, type_, ssl_);
        fnc_(mt, Udata);
        mt.close_s();
        return;
    }
    void server_baseIPV6::run_fn(server_baseIPV6 *where, svrCallbackFnIPV6 fnc_, int socket, const addrinfo cli, sock_type type_, SSL *ssl_, bool thread_, void *Udata) noexcept
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
    size_t server_baseIPV6::get_connection_len() const noexcept
    {
        return connections.use_count() - 1;
    }
    size_t server_baseIPV6::get_connection_no() const noexcept
    {
        return connection_no;
    }
}