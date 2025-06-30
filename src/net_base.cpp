#include <base.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{

    int net_base::send_m(const void *data, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
#ifdef SSL_AVILABLE
        if (type == SSL_c)
            return SSL_write(ssl, (char *)data, len);
#endif // SSL_AVILABLE
        return send(sock, (char *)data, len, 0);
    }

    int net_base::recv_m(void *buf, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
#ifdef SSL_AVILABLE
        if (type == SSL_c)
            return SSL_read(ssl, (char *)buf, len);
#endif // SSL_AVILABLE
        return recv(sock, (char *)buf, len, 0);
    }

    int net_base::close_m() noexcept
    {
        if (this_status == offline)
            return error;
#ifdef SSL_AVILABLE
        if (ctx != nullptr)
        {
            SSL_CTX_free(ctx);
            ctx = nullptr;
        }
        if (ssl != nullptr)
        {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            ssl = nullptr;
        }
#endif // SSL_AVILABLE

        this_status = offline;
        shutdown(sock, SHUT_RW);
        return close(sock);
    }

    net_base::net_base() noexcept
    {
        base_no++;
        base_len++;
        this_no = base_no;
    }
    net_base::~net_base()
    {
        base_len--;
        close_m();
    }
    size_t net_base::base_no = 0;
    size_t net_base::base_len = 0;

    size_t net_base::get_base_no() const noexcept
    {
        return base_no;
    }
    size_t net_base::get_base_len() const noexcept
    {
        return base_len;
    }
    size_t net_base::get_this_no() const noexcept
    {
        return this_no;
    }

    int net_base::send_data(const void *data, size_t len) const noexcept
    {
        if (len == 0)
            len = sizeof(data);
        return send_m(data, len);
    }
    int net_base::recv_data(void *buf, size_t len) const noexcept
    {
        return recv_m(buf, len);
    }
    std::string net_base::recv_all() const noexcept
    {
        char buffer[100] = {0};
        memset(buffer, 0, sizeof(buffer));
        std::string result;
        // receive all
        while (recv_data(buffer, sizeof(buffer) - 1) > 0)
        {
            result += buffer;
            memset(buffer, 0, sizeof(buffer));
        }
        return result;
    }
    int net_base::close_s() noexcept
    {
        return close_m();
    }

}
