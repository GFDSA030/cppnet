#include <base.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
    int net_baseIPV6::send_tcp(const void *data, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
        return send(sock, (char *)data, len, 0);
    }
    int net_baseIPV6::recv_tcp(void *buf, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
        return recv(sock, (char *)buf, len, 0);
    }
    int net_baseIPV6::close_tcp() noexcept
    {
        if (this_status == offline)
            return error;
        this_status = offline;
        shutdown(sock, SHUT_RW);
        return close(sock);
    }
#ifdef NETCPP_SSL_AVAILABLE
    int net_baseIPV6::send_ssl(const void *data, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
        return SSL_write(ssl, (char *)data, len);
    }
    int net_baseIPV6::recv_ssl(void *buf, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
        return SSL_read(ssl, (char *)buf, len);
    }
    int net_baseIPV6::close_ssl() noexcept
    {
        if (this_status == offline)
            return error;
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

        this_status = offline;
        shutdown(sock, SHUT_RW);
        return close(sock);
    }
#endif

    int net_baseIPV6::set_type(sock_type type_) noexcept
    {
        switch (type_)
        {
        case TCP_c:
            type = TCP_c;
            send_m = std::bind(&net_baseIPV6::send_tcp, this, std::placeholders::_1, std::placeholders::_2);
            recv_m = std::bind(&net_baseIPV6::recv_tcp, this, std::placeholders::_1, std::placeholders::_2);
            close_m = std::bind(&net_baseIPV6::close_tcp, this);
            break;
        case SSL_c:
#ifdef NETCPP_SSL_AVAILABLE
            type = SSL_c;
            send_m = std::bind(&net_baseIPV6::send_ssl, this, std::placeholders::_1, std::placeholders::_2);
            recv_m = std::bind(&net_baseIPV6::recv_ssl, this, std::placeholders::_1, std::placeholders::_2);
            close_m = std::bind(&net_baseIPV6::close_ssl, this);
#else
            return error;
#endif // NETCPP_SSL_AVAILABLE
            break;

        default:
            return error;
            break;
        }
        return success;
    }

    net_baseIPV6::net_baseIPV6() noexcept
    {
        netcpp_start();
        base_no++;
        base_len++;
        this_no = base_no;
    }
    net_baseIPV6::~net_baseIPV6()
    {
        base_len--;
        close_m();
        netcpp_stop();
    }
    size_t net_baseIPV6::base_no = 0;
    size_t net_baseIPV6::base_len = 0;

    size_t net_baseIPV6::get_base_no() noexcept
    {
        return base_no;
    }
    size_t net_baseIPV6::get_base_len() noexcept
    {
        return base_len;
    }
    size_t net_baseIPV6::get_this_no() const noexcept
    {
        return this_no;
    }

    int net_baseIPV6::send_data(const void *data, size_t len) const noexcept
    {
        // length must be provided for raw pointer data; sizeof(data) would return pointer size
        if (len == 0)
            return error;
        return send_m(data, len);
    }
    int net_baseIPV6::recv_data(void *buf, size_t len) const noexcept
    {
        return recv_m(buf, len);
    }

    int net_baseIPV6::send_data(const std::string &data, size_t len) const noexcept
    {
        if (len == 0)
            len = data.size();
        if (len == 0)
            return 0;
        return send_m(data.c_str(), len);
    }
    int net_baseIPV6::recv_data(std::string &buf, size_t len) const noexcept
    {
        if (len == 0)
            return 0;
        char *buffer = new char[len];
        memset(buffer, 0, len);
        int ret = recv_m(buffer, len);
        if (ret > 0)
        {
            buf.assign(buffer, ret);
        }
        delete[] buffer;
        return ret;
    }
    std::string net_baseIPV6::recv_all() const noexcept
    {
        char buffer[BUF_SIZE] = {0};
        memset(buffer, 0, BUF_SIZE);
        std::string result;
        // receive all
        int ret = 0;
        while ((ret = recv_data(buffer, BUF_SIZE - 1)) > 0)
        {
            // append exact number of bytes received (may contain nulls)
            result.append(buffer, ret);
            memset(buffer, 0, BUF_SIZE);
        }
        return result;
    }
    int net_baseIPV6::close_s() noexcept
    {
        return close_m();
    }

}
