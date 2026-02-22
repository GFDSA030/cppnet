#pragma once

#include <base.h>

namespace unet
{
    class Client : public net_base
    {
    private:
    public:
        Client() noexcept;
        /// @brief クライアントクラス
        /// @param addr_ 接続先アドレス（ドメイン可）
        /// @param type_ 接続タイプ
        /// @param port_ 接続先ポート(-1ならtype_に従う)
        Client(const char *addr_, const sock_type type_, const int port_ = -1) noexcept;
        /// @brief 接続
        /// @param addr_ 接続先アドレス（ドメイン可）
        /// @param type_ 接続タイプ
        /// @param port_ 接続先ポート(-1ならtype_に従う)
        /// @return 成功で0、エラー時は-1
        int connect_s(const char *addr_, const sock_type type_, const int port_ = -1) noexcept;
        /// @brief 接続タイプ変更
        /// @param type_ 接続タイプ
        /// @return 変更後のタイプ、エラー時はunknown
        sock_type change_type(const sock_type type_) noexcept;
        ~Client();
    };
}
