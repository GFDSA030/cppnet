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
    class server_base
    {
    private:
        static void fn2core(server_base *where, svrCallbackFn fnc_, int socket, const IPaddress cli, sock_type type_, SSL *ssl_, void *Udata) noexcept;
        std::shared_ptr<size_t> connections = std::make_shared<size_t>(0);
        std::atomic_size_t connection_no{0};
        void join_worker_threads() noexcept;

    protected:
        server_base() noexcept;
        ~server_base();

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
        std::vector<std::thread> worker_threads;
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
        /// @brief 接続タイプ変更
        /// @param type_ 接続タイプ
        /// @return 変更後のタイプ、エラー時はunknown
        sock_type change_type(const sock_type type_) noexcept;
        /// @brief サーバーリッスン(非ブロッキング)
        /// @param block ブロッキングモード(trueならブロッキング)
        /// @return 成功で0、エラー時は-1
        int listen_p(bool block = true) noexcept;
    };
    typedef Server Server_com;

}
