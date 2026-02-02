# APIリファレンス

## `base.h`

### 関数

- `int netcpp_start() noexcept;`
  - ネットワークモジュールを開始します。
- `int netcpp_stop() noexcept;`
  - ネットワークモジュールを停止します。
- `int getipaddr(const char *addr_, IPaddress &ret) noexcept;`
  - IPアドレスを取得します。
- `std::string ip2str(const IPaddress &addr) noexcept;`
  - IPアドレスを文字列に変換します。

### クラス

- `class net_base`
  - ネットワークの基本クラス。

---

## `client.h`

### クラス

- `class Client : public net_base`
  - クライアント接続を管理するクラス。
  - **コンストラクタ**
    - `Client() noexcept;`
    - `Client(const char *addr_, const sock_type type_, const int port_ = -1) noexcept;`
  - **メソッド**
    - `int connect_s(const char *addr_, const sock_type type_, const int port_ = -1) noexcept;`
      - 接続を確立します。
    - `sock_type change_type(const sock_type type_) noexcept;`
      - 接続タイプを変更します。

---

## `http.h`

### 名前空間

- `namespace unet::inline http`
  - **関数**
    - `std::string get_http_result_header(...)`
      - HTTPレスポンスヘッダを生成します。
    - `std::string get_http_request_header(...)`
      - HTTPリクエストヘッダを生成します。
    - `std::string extract_http_body(const std::string &header);`
      - HTTPボディを抽出します。

---

## `infnc.h`

### 定数

- `constexpr int BUF_SIZE = 1024;`
  - バッファサイズ。

### 関数

- `void netinit() noexcept;`
  - ネットワークを初期化します。
- `void netquit() noexcept;`
  - ネットワークを終了します。

---

## `netdefs.h`

### 定義

- `#define NETCPP_SSL_AVAILABLE`
  - SSLサポートを有効化。
- `#define NETCPP_BLOCKING`
  - ブロッキングモードを有効化。

---

## `server.h`

### クラス

- `class server_base`
  - サーバーの基本クラス。
  - **メソッド**
    - `int setUserData(void *data) noexcept;`
      - ユーザーデータを設定します。
    - `size_t get_connection_len() const noexcept;`
      - 現在の接続数を取得します。
    - `int stop() noexcept;`
      - サーバーを停止します。

---

## `udp.h`

### クラス

- `class udp_core`
  - UDP通信を管理するクラス。
  - **メソッド**
    - `int send_data(const char *addr_, const char *buf, int len);`
      - データを送信します。
    - `int recv_data(char *buf, int len);`
      - データを受信します。

---

## `unet.h`

### 概要

- このヘッダは、`base.h`, `server.h`, `client.h`, `udp.h`, `http.h` を統合します。
