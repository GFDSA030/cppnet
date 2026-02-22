#ifndef UDP_H
#define UDP_H
#include <netdefs.h>

namespace unet
{
    class udp_core // TODO: 構成考える
    {
    private:
        static size_t udp_no;

    protected:
        int send_m(const IPaddress *addr, const char *buf, int len) const noexcept;
        int recv_m(IPaddress *addr, char *buf, int len) const noexcept;
        mutable int TXsock = 0;
        int TXport = 0;
        mutable int RXsock = 0;
        int RXport = 0;

    public:
        /// @brief UDPクラス
        /// @param Tx_ 送信ポート
        /// @param Rx_ 受信ポート
        udp_core(int Tx_, int Rx_);
        udp_core();
        ~udp_core();
        /// @brief ポート設定
        /// @param Tx_ 送信ポート
        /// @param Rx_ 受信ポート
        /// @return 0で成功-1で失敗
        int set_port(int Tx_, int Rx_);
        /// @brief データ送信
        /// @param addr_ 宛先アドレス（ドメイン可）
        /// @param buf データバッファ
        /// @param len 送信長
        /// @return 0で成功-1で失敗
        int send_data(const char *addr_, const char *buf, int len);
        /// @brief データ受信
        /// @param buf 受信バッファ
        /// @param len 受信長
        /// @return 0で成功-1で失敗
        int recv_data(char *buf, int len);
        /// @brief データ受信（送信元付き）
        /// @param addr 送信元アドレス
        /// @param buf 受信バッファ
        /// @param len 受信長
        /// @return 0で成功-1で失敗
        int recv_data(IPaddress *addr, char *buf, int len);
    };

}

#endif // UDP_H
