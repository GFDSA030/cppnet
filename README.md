# cppserver_linux 日本語ドキュメント

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
git clone https://github.com/GFDSA030/cppserver_linux.git cpp-net
cd cpp-net
make
```

## クラス・定数の説明

### 通信種別

- `unet::TCP_c` : TCP 通信
- `unet::UDP_c` : UDP 通信（未テスト）
- `unet::SSL_c` : SSL 通信（OpenSSL 必須）

### 主なクラス

- `unet::net_base` : サーバ・クライアント共通の基底クラス（派生クラスとして使用）
- `unet::ServerTCP` : TCP サーバクラス
- `unet::ServerSSL` : SSL サーバクラス
- `unet::Server_com` : 複数種別対応サーバクラス（オーバーヘッド有）
- `unet::ClientTCP` : TCP クライアントクラス
- `unet::ClientSSL` : SSL クライアントクラス
- `unet::Client_com` : 複数種別対応クライアントクラス（オーバーヘッド有）

## クラス別サンプル

### unet::net_base（基底クラス）

```cpp
#include <unet.h>
#include <iostream>
int main() {
    unet::netinit();
    unet::net_base base;
    // 基本的なソケット操作が可能
    base.close_s();
    unet::netquit();
}
```

### unet::ServerTCP（TCP サーバ）

```cpp
#include <server.h>
#include <iostream>
void callback(unet::net_core &con) {
    std::string msg = "Hello TCP!";
    con.send_data(msg.c_str(), msg.size());
    con.close_s();
}
int main() {
    unet::netinit();
    unet::ServerTCP svr(9000, callback);
    svr.listen_p();
    unet::netquit();
}
```

### unet::ServerSSL（SSL サーバ）

```cpp
#include <server.h>
#include <iostream>
void callback(unet::net_core &con) {
    std::string msg = "Hello SSL!";
    con.send_data(msg.c_str(), msg.size());
    con.close_s();
}
int main() {
    unet::netinit();
    unet::ServerSSL svr(9443, callback, "server.crt", "server.key");
    svr.listen_p();
    unet::netquit();
}
```

### unet::Server_com（複数種別対応サーバ）

```cpp
#include <server.h>
#include <iostream>
void callback(unet::net_core &con) {
    std::string msg = "Hello Server_com!";
    con.send_data(msg.c_str(), msg.size());
    con.close_s();
}
int main() {
    unet::netinit();
    unet::Server_com svr(9090, callback, unet::TCP_c);
    svr.listen_p();
    unet::netquit();
}
```

### unet::ClientTCP（TCP クライアント）

```cpp
#include <client.h>
#include <iostream>
int main() {
    unet::netinit();
    unet::ClientTCP cli("localhost", 9000);
    cli.send_data("Hello TCP server", 17);
    std::cout << cli.recv_all() << std::endl;
    cli.close_s();
    unet::netquit();
}
```

### unet::ClientSSL（SSL クライアント）

```cpp
#include <client.h>
#include <iostream>
int main() {
    unet::netinit();
    unet::ClientSSL cli("localhost", 9443);
    cli.send_data("Hello SSL server", 17);
    std::cout << cli.recv_all() << std::endl;
    cli.close_s();
    unet::netquit();
}
```

### unet::Client_com（複数種別対応クライアント）

```cpp
#include <client.h>
#include <iostream>
int main() {
    unet::netinit();
    unet::Client_com cli("localhost", unet::TCP_c, 9090);
    cli.send_data("Hello Server_com", 16);
    std::cout << cli.recv_all() << std::endl;
    cli.close_s();
    unet::netquit();
}
```

## ライセンス

[zlib License](LICENSE_ja)
