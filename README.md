# cppnet ドキュメント

[English version](README_en.md)

## 概要

`cppnet` は、TCP/UDP/SSL 通信を C++ で扱うための軽量ネットワークライブラリです。  
`#include <unet.h>` で、以下の主要ヘッダをまとめて利用できます。

- `netdefs.h`
- `base.h`
- `server.h`
- `client.h`
- `udp.h`
- `http.h`

## 公開 API（主要）

### 初期化とユーティリティ

- `unet::netcpp_start()`
- `unet::netcpp_stop()`
- `unet::getipaddr(...)`
- `unet::getipaddrinfo(...)`
- `unet::ip2str(...)`

### ソケット種別

- `unet::TCP_c`
- `unet::SSL_c`
- `unet::UDP_c`

### 主要クラス

- `unet::Server`（`typedef Server_com` あり）
- `unet::Client`
- `unet::Standby`
- `unet::net_core`（Server コールバック側）
- `unet::udp_core`
- `unet::net_base`（共通ベース）

### HTTP ヘルパ

- `unet::http::get_http_request_header(...)`
- `unet::http::get_http_result_header(...)`
- `unet::http::extract_http_header(...)`
- `unet::http::extract_http_body(...)`

## 動作確認済み項目（`test2.cpp`）

`test2.cpp` では次の 11 項目を検証しています。

- HTTP ヘルパ（ヘッダ生成/抽出）
- IP 解決ユーティリティ
- `net_base` の基本動作
- `net_base::recv_data` のオーバーロード
- `Server` + `Client`（TCP）
- `Server` + `Client`（SSL）
- `Standby` クライアント（TCP）
- `Standby` クライアント（SSL）
- `Standby` サーバ（TCP）
- `Standby` サーバ（SSL）
- `udp_core` の送受信・タイムアウト・アドレス付き受信

## ビルド

### Linux (Ubuntu) 例

```bash
sudo apt install -y g++ clang make libssl-dev zlib1g-dev
```

```bash
git clone https://github.com/GFDSA030/cppnet.git cppnet
cd cppnet
make
```

`Makefile` の `IFILE` / `LFILE` は環境に応じて調整してください（特に Windows）。

## 実行

```bash
./main.out
./test.out
./test2.out
```

## 最小サンプル

### Server + Client（TCP）

```cpp
#include <unet.h>
#include <thread>
#include <chrono>
#include <iostream>

void on_conn(unet::net_core& nc, void*) {
    std::string req;
    nc.recv_data(req, 2048, 1000);
    const std::string res =
        unet::http::get_http_result_header("200 OK", "text/plain", 2) + "ok";
    nc.send_data(res);
    nc.close_s();
}

int main() {
    unet::netcpp_start();

    unet::Server server(18080, on_conn, unet::TCP_c, "server.crt", "server.key", false);
    server.listen_p(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    unet::Client client;
    client.connect_s("::1", unet::TCP_c, 18080);
    client.send_data(unet::http::get_http_request_header("GET", "/", "localhost"));
    std::cout << client.recv_all(3000) << std::endl;
    client.close_s();

    server.stop();
    unet::netcpp_stop();
}
```

### UDP（`udp_core`）

```cpp
#include <unet.h>
#include <iostream>

int main() {
    unet::netcpp_start();

    unet::udp_core udp;
    udp.set_port(18140, 18141);               // TX, RX
    udp.send_data("::1", "hello", 5);

    char buf[64] = {};
    int n = udp.recv_data(buf, sizeof(buf), 2000);
    if (n > 0) std::cout << std::string(buf, n) << std::endl;

    unet::netcpp_stop();
}
```

## 注意事項

- SSL 通信には証明書/秘密鍵ファイル（例: `server.crt`, `server.key`）が必要です。
- `test2.cpp` は IPv6 ループバック（`::1`）を使うため、実行環境で IPv6 が有効である必要があります。

## ライセンス

[zlib License](LICENSE)
