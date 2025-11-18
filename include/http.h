#ifndef HTTP_H
#define HTTP_H
#include <netdefs.h>
#include <string>

namespace unet::inline http
{
    /// @brief HTTP返信ヘッダ生成
    /// @param status ステータスコード ex:) "200 OK"
    /// @param content_type コンテンツタイプ ex:) "text/html"
    /// @param content_length コンテンツの長さ ex:) 1234
    /// @param options オプションのヘッダ ex:) "Set-Cookie: Sample_acc=accNo1\r\n"
    /// @return 生成結果
    std::string get_http_result_header(const std::string &status, const std::string &content_type, size_t content_length, const std::string &options = "");
    /// @brief HTTPリクエストヘッダ生成
    /// @param method HTTPメソッド ex:) "GET", "POST"
    /// @param path リクエストパス ex:) "/index.html"
    /// @param host ホスト名 ex:) "example.com"
    /// @param user_agent ユーザーエージェント
    /// @return 生成結果
    std::string get_http_request_header(const std::string &method, const std::string &path, const std::string &host,
                                        const std::string &user_agent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36");
    /// @brief HTTPヘッダからボディを抽出
    /// @param header HTTPヘッダ
    /// @return ボディ部分
    std::string extract_http_body(const std::string &header);
    /// @brief HTTPヘッダを抽出
    /// @param header HTTPヘッダ
    /// @return ヘッダ部分
    std::string extract_http_header(const std::string &header);
}

#endif // HTTP_H