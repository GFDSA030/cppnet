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
    int net_base::recv_tcp(void *buf, size_t len, int32_t timeout) const noexcept
    {
        if (this_status == offline)
            return error;

        // Use select for timeout handling
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        struct timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        int sel = select(sock + 1, &readfds, NULL, NULL, timeout >= 0 ? &tv : NULL);
        if (sel > 0)
        {
            return recv(sock, (char *)buf, len, 0);
        }
        else if (sel == 0)
        {
            // Timeout occurred
            return 0;
        }
        else
        {
            // Error occurred
            return error;
        }
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
    int net_base::recv_ssl(void *buf, size_t len, int32_t timeout) const noexcept
    {
        if (this_status == offline)
            return error;

        // Use select for timeout handling
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        struct timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        int sel = select(sock + 1, &readfds, NULL, NULL, timeout >= 0 ? &tv : NULL);
        if (sel > 0)
        {
            return SSL_read(ssl, (char *)buf, len);
        }
        else if (sel == 0)
        {
            // Timeout occurred
            return 0;
        }
        else
        {
            // Error occurred
            return error;
        }
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
#endif

    int net_base::set_type(sock_type type_) noexcept
    {
        switch (type_)
        {
        case TCP_c:
            type = TCP_c;
            send_m = std::bind(&net_base::send_tcp, this, std::placeholders::_1, std::placeholders::_2);
            recv_m = std::bind(&net_base::recv_tcp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            close_m = std::bind(&net_base::close_tcp, this);
            break;
        case SSL_c:
#ifdef NETCPP_SSL_AVAILABLE
            type = SSL_c;
            send_m = std::bind(&net_base::send_ssl, this, std::placeholders::_1, std::placeholders::_2);
            recv_m = std::bind(&net_base::recv_ssl, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
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
        send_m = std::bind(&net_base::send_tcp, this, std::placeholders::_1, std::placeholders::_2);
        recv_m = std::bind(&net_base::recv_tcp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        close_m = std::bind(&net_base::close_tcp, this);
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
        // length must be provided for raw pointer data; sizeof(data) would return pointer size
        if (len == 0)
            return error;
        return send_m(data, len);
    }
    int net_base::recv_data(void *buf, size_t len, int32_t timeout) const noexcept
    {
        return recv_m(buf, len, timeout);
    }

    int net_base::send_data(const std::string &data, size_t len) const noexcept
    {
        if (len == 0)
            len = data.size();
        if (len == 0)
            return 0;
        return send_m(data.c_str(), len);
    }
    int net_base::recv_data(std::string &buf, size_t len, int32_t timeout) const noexcept
    {
        if (len == 0)
            return 0;
        char *buffer = new char[len];
        memset(buffer, 0, len);
        int ret = recv_m(buffer, len, timeout);
        if (ret > 0)
        {
            buf.assign(buffer, ret);
        }
        delete[] buffer;
        return ret;
    }
    std::string net_base::recv_all(int32_t timeout) const noexcept
    {
        char buffer[BUF_SIZE];
        std::string result;

        if (this_status == offline)
            return result;

        // Use select to wait for readability with a short timeout so that
        // non-blocking sockets don't cause a busy loop or premature exit.
#if defined(_WIN32) || defined(__MINGW32__)
        SOCKET s = (SOCKET)sock;
#else
        int s = sock;
#endif

        for (;;)
        {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(s, &readfds);
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 200 * 1000; // 200 ms

            int sel = select(s + 1, &readfds, NULL, NULL, &tv);
            if (sel < 0)
            {
                // select error
                break;
            }
            if (sel == 0)
            {
                // timeout with no data -> stop reading
                break;
            }

            // socket is readable
            int ret = recv_data(buffer, BUF_SIZE, timeout);
            if (ret > 0)
            {
                result.append(buffer, ret);
                // continue to try to read more while data may be available
                continue;
            }
            else if (ret == 0)
            {
                // connection closed by peer
                break;
            }
            else
            {
                // ret < 0, error
#ifdef NETCPP_SSL_AVAILABLE
                if (type == SSL_c && ssl != nullptr)
                {
                    int ssl_err = SSL_get_error(ssl, ret);
                    if (ssl_err == SSL_ERROR_WANT_READ || ssl_err == SSL_ERROR_WANT_WRITE)
                    {
                        // no data now, wait again
                        continue;
                    }
                    // otherwise treat as fatal
                }
#endif
                break;
            }
        }
        return result;
    }
    int net_base::close_s() noexcept
    {
        return close_m();
    }

}
