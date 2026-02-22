#include <server.h>
#include <infnc.h>
#include <thread>
namespace unet
{
    server_base::server_base() noexcept {
    };

    void server_base::cleanup_worker_threads_locked(std::vector<worker_thread_entry> &finished) noexcept
    {
        auto it = worker_threads.begin();
        while (it != worker_threads.end())
        {
            if (it->done && it->done->load(std::memory_order_acquire))
            {
                finished.emplace_back(std::move(*it));
                it = worker_threads.erase(it);
                continue;
            }
            ++it;
        }
    }

    void server_base::cleanup_worker_threads() noexcept
    {
        std::vector<worker_thread_entry> finished;
        {
            std::lock_guard<std::mutex> lock(worker_threads_mtx);
            cleanup_worker_threads_locked(finished);
        }
        for (auto &entry : finished)
        {
            if (entry.worker.joinable() &&
                entry.worker.get_id() != std::this_thread::get_id())
            {
                entry.worker.join();
            }
        }
        // After a burst, erase() keeps vector capacity; compact occasionally.
        {
            std::lock_guard<std::mutex> lock(worker_threads_mtx);
            const size_t size = worker_threads.size();
            const size_t cap = worker_threads.capacity();
            if ((size == 0 && cap > 64) || (cap > 256 && cap > size * 4))
            {
                worker_threads.shrink_to_fit();
            }
        }
    }

    void server_base::join_worker_threads() noexcept
    {
        std::vector<worker_thread_entry> local_threads;
        {
            std::lock_guard<std::mutex> lock(worker_threads_mtx);
            local_threads.swap(worker_threads);
        }
        for (auto &entry : local_threads)
        {
            if (entry.worker.joinable() &&
                entry.worker.get_id() != std::this_thread::get_id())
            {
                entry.worker.join();
            }
        }
        {
            std::lock_guard<std::mutex> lock(worker_threads_mtx);
            worker_threads.shrink_to_fit();
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
        where->connections.fetch_sub(1, std::memory_order_relaxed);
        return;
    }
    void server_base::run_fn(server_base *where, svrCallbackFn fnc_, int socket, const IPaddress cli, sock_type type_, SSL *ssl_, bool thread_, void *Udata) noexcept
    {
        where->connections.fetch_add(1, std::memory_order_relaxed);
        where->connection_no.fetch_add(1, std::memory_order_relaxed);
        if (thread_)
        {
            where->cleanup_worker_threads();
            std::shared_ptr<std::atomic_bool> done =
                std::make_shared<std::atomic_bool>(false);
            std::thread worker([where, fnc_, socket, cli, type_, ssl_, Udata, done]() noexcept {
                fn2core(where, fnc_, socket, cli, type_, ssl_, Udata);
                done->store(true, std::memory_order_release);
            });
            std::lock_guard<std::mutex> lock(where->worker_threads_mtx);
            where->worker_threads.emplace_back(
                worker_thread_entry{std::move(worker), std::move(done)});
        }
        else
        {
            fn2core(where, fnc_, socket, cli, type_, ssl_, Udata);
        }
    }
    size_t server_base::get_connection_len() const noexcept
    {
        return connections.load(std::memory_order_relaxed);
    }
    size_t server_base::get_connection_no() const noexcept
    {
        return connection_no.load(std::memory_order_relaxed);
    }
}
