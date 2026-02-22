#include <server.h>
#include <infnc.h>
#include <thread>
namespace unet
{
    server_base::server_base() noexcept {
    };

    void server_base::join_worker_threads() noexcept
    {
        std::vector<std::thread> local_threads;
        {
            std::lock_guard<std::mutex> lock(worker_threads_mtx);
            local_threads.swap(worker_threads);
        }
        for (auto &t : local_threads)
        {
            if (t.joinable() && t.get_id() != std::this_thread::get_id())
                t.join();
        }
    }

    server_base::~server_base()
    {
        stop();
        netcpp_stop();
    };
    int server_base::stop() noexcept
    {
        cont = false;
        if (sock >= 0)
        {
            shutdown(sock, SHUT_RW);
            close(sock);
            sock = -1;
        }
        if (listen_thread.joinable() &&
            listen_thread.get_id() != std::this_thread::get_id())
        {
            listen_thread.join();
        }
        join_worker_threads();
#ifdef NETCPP_SSL_AVAILABLE
        // ctx can still be referenced by listen/worker threads until they join.
        if (ctx != nullptr)
        {
            SSL_CTX_free(ctx);
            ctx = nullptr;
        }
#endif
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
        size_t prev = where->connection_no.fetch_sub(1, std::memory_order_relaxed);
        if (prev == 0)
            where->connection_no.store(0, std::memory_order_relaxed);
        return;
    }
    void server_base::run_fn(server_base *where, svrCallbackFn fnc_, int socket, const IPaddress cli, sock_type type_, SSL *ssl_, bool thread_, void *Udata) noexcept
    {
        where->connection_no.fetch_add(1, std::memory_order_relaxed);
        if (thread_)
        {
            std::thread worker(fn2core, where, fnc_, socket, cli, type_, ssl_, Udata);
            std::lock_guard<std::mutex> lock(where->worker_threads_mtx);
            where->worker_threads.emplace_back(std::move(worker));
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
        return connection_no.load(std::memory_order_relaxed);
    }
}
