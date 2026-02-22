#pragma once

#include <base.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
namespace unet
{
    typedef void (*svrCallbackFn)(net_core &, void *);
    struct worker_thread_entry
    {
        std::thread worker;
        std::shared_ptr<std::atomic_bool> done;
    };
    class server_base
    {
    private:
        static void fn2core(server_base *where, svrCallbackFn fnc_, int socket, const IPaddress cli, sock_type type_, SSL *ssl_, void *Udata) noexcept;
        std::atomic_size_t connections{0};   // 現在接続数
        std::atomic_size_t connection_no{0}; // 過去合計接続数
        void join_worker_threads() noexcept;
        void cleanup_worker_threads_locked(std::vector<worker_thread_entry> &finished) noexcept;

    protected:
        server_base() noexcept;
        ~server_base();
        void cleanup_worker_threads() noexcept;

        static void run_fn(server_base *where, svrCallbackFn fnc_, int socket, const IPaddress cli, sock_type type_, SSL *ssl_, bool thread_, void *Udata) noexcept;

        int sock = -1;
        IPaddress addr = {};
        svrCallbackFn fnc = nullptr;
        sock_type type = TCP_c;
        SSL_CTX *ctx = nullptr;
        bool thread_use = true;
        std::atomic_bool cont{true};
        std::thread listen_thread;
        std::mutex worker_threads_mtx;
        std::vector<worker_thread_entry> worker_threads;
        void *UserData = nullptr;

    public:
        /// @brief ユーザーデータ設定
        /// @param data ユーザーデータポインタ
        /// @return エラー時は-1、成功で0
        int setUserData(void *data) noexcept;
        /// @brief 接続中のコネクション数取得
        /// @return 現在のコネクション数
        size_t get_connection_len() const noexcept;
        /// @brief 接続されたコネクション数取得
        /// @return 今までの接続数
        size_t get_connection_no() const noexcept;
        /// @brief サーバー停止
        /// @return エラー時は-1、成功で0
        int stop() noexcept;
    };

    class Server : public server_base
    {
    private:
        int listen_m() noexcept;

    public:
        /// @brief サーバークラス
        /// @param port_ 待ち受けポート
        /// @param fnc_ コールバック関数
        /// @param type_ 接続タイプ
        /// @param crt 証明書ファイル
        /// @param pem 鍵ファイル
        /// @param thread_ スレッド使用
        Server(int port_, svrCallbackFn fnc_, sock_type type_ = TCP_c, const char *crt = "", const char *pem = "", bool thread_ = true) noexcept;
        ~Server();
        /// @brief サーバーリッスン(非ブロッキング)
        /// @param block ブロッキングモード(trueならブロッキング)
        /// @return 成功で0、エラー時は-1
        int listen_p(bool block = true) noexcept;
    };
    typedef Server Server_com;

}
