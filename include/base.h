#pragma once
#include <netdefs.h>
#include <string>
#include <functional>

namespace unet
{

    int netcpp_start() noexcept;
    int netcpp_stop() noexcept;
    int getipaddr(const char *addr_, IPaddress &ret) noexcept;
    IPaddress getipaddr(const char *addr_) noexcept;
    int getipaddrinfo(const char *addr_, int port_, IPaddress &ret, sock_type type_ = TCP_c) noexcept;
    IPaddress getipaddrinfo(const char *addr_, int port_, sock_type type_ = TCP_c) noexcept;
    std::string ip2str(const IPaddress &addr) noexcept;
    class net_base
    {
    private:
        static size_t base_no;
        static size_t base_len;
        size_t this_no = 0;
        int send_tcp(const void *data, size_t len) const noexcept;
        int recv_tcp(void *buf, size_t len, int32_t timeout) const noexcept;
        int close_tcp() noexcept;

        int send_cry(const void *data, size_t len) const noexcept;
        int recv_cry(void *buf, size_t len, int32_t timeout) const noexcept;
        int close_cry() noexcept;
#ifdef NETCPP_SSL_AVAILABLE
        int send_ssl(const void *data, size_t len) const noexcept;
        int recv_ssl(void *buf, size_t len, int32_t timeout) const noexcept;
        int close_ssl() noexcept;
#endif // NETCPP_SSL_AVAILABLE

    protected:
        int sock = -1;
        IPaddress addr = {};
        sock_type type = TCP_c;
        status this_status = offline;
        SSL *ssl = nullptr;
        SSL_CTX *ctx = nullptr;
        net_base() noexcept;
        ~net_base();

        std::function<int(const void *, size_t)> send_m = std::bind(&net_base::send_tcp, this, std::placeholders::_1, std::placeholders::_2);
        std::function<int(void *, size_t, int32_t)> recv_m = std::bind(&net_base::recv_tcp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        std::function<int()> close_m = std::bind(&net_base::close_tcp, this);

        int set_type(sock_type type_) noexcept;

        static size_t get_base_no() noexcept;
        static size_t get_base_len() noexcept;
        size_t get_this_no() const noexcept;

    public:
        /// @brief データ送信
        /// @param data データポインタ
        /// @param len データ長
        /// @return 送信バイト数、エラー時は-1
        int send_data(const void *data, size_t len) const noexcept;
        /// @brief データ受信
        /// @param buf データバッファ
        /// @param len 受信バッファ長
        /// @param timeout タイムアウト時間(ms)、-1で無限待機
        /// @return 受信バイト数、エラー時は-1
        int recv_data(void *buf, size_t len, int32_t timeout = -1) const noexcept;
        /// @brief Stringデータ送信
        /// @param data データ
        /// @param len データ長(0ならdata.size())
        /// @return 送信バイト数、エラー時は-1
        int send_data(const std::string &data, size_t len = 0) const noexcept;
        /// @brief Stringデータ受信
        /// @param buf データバッファ
        /// @param len 受信バッファ長
        /// @param timeout タイムアウト時間(ms)、-1で無限待機
        /// @return 受信バイト数、エラー時は-1
        int recv_data(std::string &buf, size_t len, int32_t timeout = -1) const noexcept;
        /// @brief 全データ受信 通信終了時まで
        /// @return 受信データ、エラー時は空文字
        std::string recv_all(int32_t timeout = -1) const noexcept;
        /// @brief ソケットクローズ
        /// @return クローズ成功で0、エラー時は-1
        int close_s() noexcept;
    };

    class net_core : public net_base
    {
    private:
    public:
        net_core(int socket, const IPaddress cli, sock_type type_ = TCP_c, SSL *ssl_ = nullptr) noexcept;
        ~net_core();

        IPaddress remote() const noexcept;
    };

    class Standby : public net_base
    {
    private:
        int port = 0;
        int svScok = -1;

    public:
        Standby(int port_, const sock_type type_ = TCP_c) noexcept;
        ~Standby();
        int set(int port_, const sock_type type_ = TCP_c) noexcept;
        int accept_s(const char *crt = "", const char *pem = "") noexcept;
        IPaddress get_addr() const noexcept;
        int connect_s(const char *addr_) noexcept;
        int close_s() noexcept;
    };

}
