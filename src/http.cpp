#include <http.h>

namespace unet::inline http
{
    std::string get_http_result_header(const std::string &status, const std::string &content_type, size_t content_length, const std::string &options)
    {
        return "HTTP/1.1 " + status + "\r\n" +
               "Content-Type: " + content_type + "\r\n" +
               "Content-Length: " + std::to_string(content_length) + "\r\n" +
               options + // オプションのヘッダを追加
               "Connection: close\r\n\r\n";
    }
    std::string get_http_request_header(const std::string &method, const std::string &path, const std::string &host, const std::string &user_agent)
    {
        return method + " " + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: " + user_agent + "\r\n" +
               "Accept: */*\r\n" +
               "Connection: close\r\n\r\n";
    }
    std::string extract_http_body(const std::string &header)
    {
        size_t pos = header.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            return header.substr(pos + 4); // ボディはヘッダの後にある
        }
        return ""; // ヘッダが不正な場合は空文字を返す
    }
    std::string extract_http_header(const std::string &header)
    {
        size_t pos = header.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            return header.substr(0, pos + 2); // ヘッダはボディの前にある
        }
        return ""; // ヘッダが不正な場合は空文字を返す
    }
}