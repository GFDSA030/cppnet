#include <base.h>
#include <infnc.h>
#include <string>
#include <thread>
namespace unet
{
    int net_base::send_tcp(const void *data, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
        return send(sock, (char *)data, len, 0);
    }
    int net_base::recv_tcp(void *buf, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
        return recv(sock, (char *)buf, len, 0);
    }
    int net_base::close_tcp() noexcept
    {
        if (this_status == offline)
            return error;
        this_status = offline;
        shutdown(sock, SHUT_RW);
        return close(sock);
    }
#ifdef NETCPP_SSL_AVAILABLE
    int net_base::send_ssl(const void *data, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
        return SSL_write(ssl, (char *)data, len);
    }
    int net_base::recv_ssl(void *buf, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
        return SSL_read(ssl, (char *)buf, len);
    }
    int net_base::close_ssl() noexcept
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
#endif // NETCPP_SSL_AVAILABLE
    /*
    int net_base::send_m(const void *data, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
#ifdef NETCPP_SSL_AVAILABLE
        if (type == SSL_c)
            return SSL_write(ssl, (char *)data, len);
#endif // NETCPP_SSL_AVAILABLE
        return send(sock, (char *)data, len, 0);
    }

    int net_base::recv_m(void *buf, size_t len) const noexcept
    {
        if (this_status == offline)
            return error;
#ifdef NETCPP_SSL_AVAILABLE
        if (type == SSL_c)
            return SSL_read(ssl, (char *)buf, len);
#endif // NETCPP_SSL_AVAILABLE
        return recv(sock, (char *)buf, len, 0);
    }

    int net_base::close_m() noexcept
    {
        if (this_status == offline)
            return error;
#ifdef NETCPP_SSL_AVAILABLE
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
#endif // NETCPP_SSL_AVAILABLE

        this_status = offline;
        shutdown(sock, SHUT_RW);
        return close(sock);
    }
    */
    int net_base::set_type(sock_type type_) noexcept
    {
        switch (type_)
        {
        case TCP_c:
            type = TCP_c;
            send_m = std::bind(&net_base::send_tcp, this, std::placeholders::_1, std::placeholders::_2);
            recv_m = std::bind(&net_base::recv_tcp, this, std::placeholders::_1, std::placeholders::_2);
            close_m = std::bind(&net_base::close_tcp, this);
            break;
        case SSL_c:
#ifdef NETCPP_SSL_AVAILABLE
            type = SSL_c;
            send_m = std::bind(&net_base::send_ssl, this, std::placeholders::_1, std::placeholders::_2);
            recv_m = std::bind(&net_base::recv_ssl, this, std::placeholders::_1, std::placeholders::_2);
            close_m = std::bind(&net_base::close_ssl, this);
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

    net_base::net_base() noexcept
    {
        netcpp_start();
        base_no++;
        base_len++;
        this_no = base_no;
    }
    net_base::~net_base()
    {
        base_len--;
        close_m();
        netcpp_stop();
    }
    size_t net_base::base_no = 0;
    size_t net_base::base_len = 0;

    size_t net_base::get_base_no() noexcept
    {
        return base_no;
    }
    size_t net_base::get_base_len() noexcept
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

    int net_base::send_data(const std::string &data, size_t len) const noexcept
    {
        if (len == 0)
            len = data.size();
        if (len == 0)
            return 0;
        return send_m(data.c_str(), len);
    }
    int net_base::recv_data(std::string &buf, size_t len) const noexcept
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
    std::string net_base::recv_all() const noexcept
    {
        char buffer[BUF_SIZE] = {0};
        memset(buffer, 0, BUF_SIZE);
        std::string result;
        // receive all
        while (recv_data(buffer, BUF_SIZE - 1) > 0)
        {
            result += buffer;
            memset(buffer, 0, BUF_SIZE);
        }
        return result;
    }
    int net_base::close_s() noexcept
    {
        return close_m();
    }

}
