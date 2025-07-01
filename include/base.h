#ifndef BASE
#define BASE
#include <netdefs.h>
#include <string>

namespace unet
{

    void netinit() noexcept;
    void netquit() noexcept;
    int getipaddr(const char *addr_, struct sockaddr_in &ret) noexcept;
    int getipaddrinfo(const char *addr_, int port_, struct sockaddr_in &ret, sock_type type_ = TCP_c) noexcept;
    int getipaddrinfo(const char *addr_, int port_, struct addrinfo *ret, sock_type type_ = TCP_c) noexcept;
    class net_base
    {
    private:
        static size_t base_no;
        static size_t base_len;
        size_t this_no = 0;

    protected:
        int sock = 0;
        // int port = 0;
        struct sockaddr_in addr = {};
        sock_type type = TCP_c;
        status this_status = offline;
        SSL *ssl = nullptr;
        SSL_CTX *ctx = nullptr;

        int send_m(const void *data, size_t len) const noexcept;
        int recv_m(void *buf, size_t len) const noexcept;
        int close_m() noexcept;
        net_base() noexcept;
        ~net_base();

        size_t get_base_no() const noexcept;
        size_t get_base_len() const noexcept;
        size_t get_this_no() const noexcept;

    public:
        int send_data(const void *data, size_t len = 0) const noexcept;
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

}
#endif // BASE
