# cppnet 日本語ドキュメント

[English version](README_en.md)

## 概要

C++で TCP/UDP/SSL 通信サーバ・クライアントを簡単に構築できるクロスプラットフォームネットワークライブラリです。

## 特徴

- Windows/Linux/macOS 対応
- TCP/UDP/SSL 通信をサポート
- シンプルな API 設計
- OpenSSL による SSL 通信対応
- サーバ・クライアント両方の実装が可能

## サポート環境

- Windows（MinGW, MSVC）
- Ubuntu（gcc, clang）
- macOS（未検証）

## ビルド環境構築

```bash
apt install -y clang gcc libssl-dev make
```

## ビルド方法

```bash
git clone https://github.com/GFDSA030/cppnet.git cppnet
```

Makefile に openssl のパスを設定する必要があります。Makefile の`IFILE`にヘッダへのパスを`LFILE`にライブラリへのパスを適切に設定してください。

```bash
cd cppnet
make
```

## クラス・定数の説明

### 通信種別

- `unet::TCP_c` : TCP 通信
- `unet::UDP_c` : UDP 通信（未テスト）
- `unet::SSL_c` : SSL 通信（OpenSSL 必須）

### 主なクラス

- `unet::net_core` : サーバ共通の通信クラス
- `unet::ServerTCP` : TCP サーバクラス
- `unet::ServerSSL` : SSL サーバクラス
- `unet::Server` : 複数種別対応サーバクラス
- `unet::ClientTCP` : TCP クライアントクラス
- `unet::ClientSSL` : SSL クライアントクラス
- `unet::Client` : 複数種別対応クライアントクラス

## クラス別サンプル

### unet::ServerTCP（TCP サーバ）

```cpp
#include <server.h>
#include <iostream>
void callback(unet::net_core &con, void *Udata) {
    std::string msg = "Hello TCP!";
    con.send_data(msg.c_str(), msg.size());
    con.close_s();
}
int main() {
    unet::ServerTCP svr(9000, callback);
    svr.listen_p();
}
```

### unet::ServerSSL（SSL サーバ）

```cpp
#include <server.h>
#include <iostream>
void callback(unet::net_core &con, void *Udata) {
    std::string msg = "Hello SSL!";
    con.send_data(msg.c_str(), msg.size());
    con.close_s();
}
int main() {
    unet::ServerSSL svr(9443, callback, "server.crt", "server.key");
    svr.listen_p();
}
```

### unet::Server_com（複数種別対応サーバ）

```cpp
#include <server.h>
#include <iostream>
void callback(unet::net_core &con, void *Udata) {
    std::string msg = "Hello Server_com!";
    con.send_data(msg.c_str(), msg.size());
    con.close_s();
}
int main() {
    unet::Server_com svr(9090, callback, unet::TCP_c);
    svr.listen_p();
}
```

### unet::ClientTCP（TCP クライアント）

```cpp
#include <client.h>
#include <iostream>
int main() {
    unet::ClientTCP cli("localhost", 9000);
    cli.send_data("Hello TCP server", 17);
    std::cout << cli.recv_all() << std::endl;
    cli.close_s();
}
```

### unet::ClientSSL（SSL クライアント）

```cpp
#include <client.h>
#include <iostream>
int main() {
    unet::ClientSSL cli("localhost", 9443);
    cli.send_data("Hello SSL server", 17);
    std::cout << cli.recv_all() << std::endl;
    cli.close_s();
}
```

### unet::Client_com（複数種別対応クライアント）

```cpp
#include <client.h>
#include <iostream>
int main() {
    unet::Client_com cli("localhost", unet::TCP_c, 9090);
    cli.send_data("Hello Server_com", 16);
    std::cout << cli.recv_all() << std::endl;
    cli.close_s();
}
```

### unet::ServerUDP（UDP サーバ）

```cpp
#include <server.h>
#include <iostream>
int main() {
    unet::ServerUDP svr(8000);
    char buf[1024];
    int len = svr.recv_data(buf, sizeof(buf));
    std::cout << "受信: " << std::string(buf, len) << std::endl;
    svr.send_data("Hello UDP!", 10);
    svr.close_s();
}
```

### unet::ClientUDP（UDP クライアント）

```cpp
#include <client.h>
#include <iostream>
int main() {
    unet::ClientUDP cli("localhost", 8000);
    cli.send_data("Hello UDP server", 17);
    char buf[1024];
    int len = cli.recv_data(buf, sizeof(buf));
    std::cout << std::string(buf, len) << std::endl;
    cli.close_s();
}
```

## ライセンス

[zlib License](LICENSE)
