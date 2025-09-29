#ifndef BASE
#define BASE
#include <netdefs.h>
#include <string>
#include <functional>

namespace unet
{

    int netcpp_start() noexcept;
    int netcpp_stop() noexcept;
    int getipaddr(const char *addr_, struct sockaddr_in &ret) noexcept;
    int getipaddrinfo(const char *addr_, int port_, addrinfo &ret, sock_type type_ = TCP_c) noexcept;
    class net_base
    {
    private:
        static size_t base_no;
        static size_t base_len;
        size_t this_no = 0;
        int send_tcp(const void *data, size_t len) const noexcept;
        int recv_tcp(void *buf, size_t len) const noexcept;
        int close_tcp() noexcept;
#ifdef NETCPP_SSL_AVAILABLE
        int send_ssl(const void *data, size_t len) const noexcept;
        int recv_ssl(void *buf, size_t len) const noexcept;
        int close_ssl() noexcept;
#endif // NETCPP_SSL_AVAILABLE

    protected:
        int sock = 0;
        // int port = 0;
        struct sockaddr_in addr = {};
        sock_type type = TCP_c;
        status this_status = offline;
        SSL *ssl = nullptr;
        SSL_CTX *ctx = nullptr;

        std::function<int(const void *, size_t)> send_m = std::bind(&net_base::send_tcp, this, std::placeholders::_1, std::placeholders::_2);
        std::function<int(void *, size_t)> recv_m = std::bind(&net_base::recv_tcp, this, std::placeholders::_1, std::placeholders::_2);
        std::function<int()> close_m = std::bind(&net_base::close_tcp, this);
        // int send_m(const void *data, size_t len) const noexcept;
        // int recv_m(void *buf, size_t len) const noexcept;
        // int close_m() noexcept;
        net_base() noexcept;
        ~net_base();

        int set_type(sock_type type_) noexcept;

        static size_t get_base_no() noexcept;
        static size_t get_base_len() noexcept;
        size_t get_this_no() const noexcept;

    public:
        int send_data(const void *data, size_t len) const noexcept;
        int recv_data(void *buf, size_t len) const noexcept;
        int send_data(const std::string &data, size_t len = 0) const noexcept;
        int recv_data(std::string &buf, size_t len) const noexcept;
        std::string recv_all() const noexcept;
        int close_s() noexcept;
    };
    class net_core : public net_base
    {
    private:
    public:
        net_core(int socket, const struct sockaddr_in cli, sock_type type_ = TCP_c, SSL *ssl_ = nullptr) noexcept;
        ~net_core();

        struct sockaddr_in remote() const noexcept;
    };

    class Standby : net_base // どっちにもなる
    {
    private:
        int port = 0;
        int svScok = 0;

    public:
        Standby(int port_, const sock_type type_ = TCP_c) noexcept;
        ~Standby();
        int set(int port_, const sock_type type_ = TCP_c) noexcept;
        int accept_s(const char *crt = "", const char *pem = "") noexcept;
        int connect_s(const char *addr_) noexcept;
        int close_s() noexcept;
        sock_type change_type(const sock_type type_) noexcept;
    };

}
#endif // BASE
