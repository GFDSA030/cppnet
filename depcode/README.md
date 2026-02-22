# frag モジュールドキュメント

このディレクトリは、暗号・ハッシュ・エンコード・UUID・端末入力・数値計算などの小さなユーティリティ群です。

## 構成

- 暗号
  - `aes.h`: AES-128 / AES-256 (CBC + PKCS#7)
  - `rsa.h`: RSA鍵生成 / OAEP暗号化 / PEM入出力
  - `ecdsa.h`: ECDSA(P-256) 署名・検証 / PEM入出力
  - `cryptaes.h`: ソケット通信用の暗号ラッパ
- ハッシュ
  - `md5.h`, `md5c.h`: MD5
  - `sha.h`: SHA-1 / SHA-224 / SHA-256 / SHA-384 / SHA-512
- エンコード
  - `baseN.h`: Base16/32/64 と汎用 baseN
  - `conv.h`: `std::string` と `std::vector<uint8_t>` 変換
- 圧縮
  - `complession.h`: raw DEFLATE 圧縮・展開
- 端末/環境
  - `D_getch.h`: クロスプラットフォーム `getch` / `kbhit`
  - `D_cpuid.h`: CPU/SMBIOS情報・デバイス指紋
- 識別子
  - `uuid.h`: UUID v1/v4/v7 と短縮表現
- 数値計算
  - `solveWeighted.h`: 多項式近似 / Huberロバスト近似
- 文字列変換
  - `wchar-char.h`: `wstring` <-> `string`

## 依存関係

- 必須
  - C++17 以上
  - Boost.Multiprecision (`cpp_int`) ※ `rsa.h`, `ecdsa.h`
- 任意/環境依存
  - `zlib` ※ `complession.h`
  - Windows: `ws2_32.lib`, `iphlpapi.lib`（実行環境により必要）

## 主要API

### AES (`aes.h`)

- `cryptASM::AES128`
- `cryptASM::AES256`

主なメソッド:

- `encrypt(const std::vector<uint8_t>&)`
- `decrypt(const std::vector<uint8_t>&)`

仕様:

- ブロックサイズ: 16 byte
- モード: CBC
- パディング: PKCS#7
- 返却暗号文: `IV(16byte) + ciphertext`

注意:

- 鍵長不正時は `std::runtime_error`
- パディング不正時は復号で例外

### RSA (`rsa.h`)

- クラス: `cryptASM::RSA`
- 既定公開指数: `65537`

主なメソッド:

- `generate_keys(size_t bits = 2048)`
- `encrypt(...)`, `decrypt(...)`
- `export_public_key_pem()`, `export_private_key_pem()`
- `import_public_key_pem(...)`, `import_private_key_pem(...)`

仕様:

- OAEP: SHA-256 + MGF1
- PEM: PKCS#1形式

注意:

- OAEPのサイズ制限を超える平文は例外
- 鍵生成は重い処理

### ECDSA (`ecdsa.h`)

- クラス: `cryptASM::ECDSA`
- 曲線: secp256r1 (P-256)

主なメソッド:

- `generate_keys()`
- `sign(...)` (DER: `SEQUENCE{r,s}`)
- `verify(...)`
- `export_private_key_pem()`, `export_public_key_pem()`
- `import_private_key_pem(...)`, `import_public_key_pem(...)`

### ソケット暗号 (`cryptaes.h`)

- 名前空間: `unet::cry`

主な関数:

- `connect_crypt`, `accept_crypt`
- `send_crypt`, `recv_crypt`
- `shutdown_crypt`, `close_crypt`

仕様:

- 接続時: RSAハンドシェイク
- セッション鍵: AES-256キー(32byte)
- 通信データ: AES-256-CBCで暗号化

注意:

- 内部にソケットfd単位の静的マップ状態を保持
- 同一fd再利用時のライフサイクル管理に注意

### ハッシュ (`md5.h`, `sha.h`)

- MD5: `MD5::hash`(バイナリ16byte), `MD5::str`(16進文字列)
- SHA1/256/512系:
  - `hash...`: バイナリダイジェスト
  - `str...`: 16進文字列

### BaseN (`baseN.h`)

- 固定API:
  - `base16_encode/decode`
  - `base32_encode/decode`
  - `base64_encode/decode`
- 汎用API:
  - `baseN_encode/decode(..., table)`

注意:

- Base16/32/64は一般的なテーブル実装
- 汎用baseNはテーブル文字集合に依存

### 圧縮 (`complession.h`)

- `complession(const std::string&)`
- `decomplession(const std::string&)`

仕様:

- 形式: raw DEFLATE (`windowBits = -15`)

注意:

- zlib/zlib wrapper 形式ではない

### UUID (`uuid.h`)

- `uuid::GenV1()`, `uuid::GenV4()`, `uuid::GenV7()`
- `uuid::str()` (標準UUID表示)
- `uuid::encode()` / `uuidshort::decode()` (短縮表現)

### CPU/SMBIOS (`D_cpuid.h`)

- `CPUInfo get_cpu_info()`
- `SMBIOSInfo get_smbios()`
- `std::string generate_device_fingerprint()`

仕様:

- 指紋はCPU情報 + SMBIOS情報を連結し `SHA256` 化

### 数値近似 (`solveWeighted.h`)

- `polyFit(points, degree)`
- `robustFit(points, degree, iter=20, delta=1.0)`

### 補助 (`conv.h`, `D_getch.h`, `wchar-char.h`)

- `str2vec`, `vec2str`
- `D_getch`, `D_kbhit`
- `narrow`, `widen`

## ミニサンプル

```cpp
#include "frag/aes.h"
#include "frag/conv.h"

std::string plain = "hello";
std::vector<uint8_t> key(32, 0x11);
cryptASM::AES256 aes(key);

auto enc = aes.encrypt(str2vec(plain));
auto dec = vec2str(aes.decrypt(enc));
```

```cpp
#include "frag/rsa.h"
#include "frag/conv.h"

cryptASM::RSA rsa;
rsa.generate_keys(2048);

auto c = rsa.encrypt(str2vec("secret"));
auto p = vec2str(rsa.decrypt(c));
```

## セキュリティ上の注意

- 研究/学習用途としては有用ですが、本番利用時は以下を検討してください。
  - 鍵・乱数の品質評価
  - サイドチャネル耐性
  - 例外時の情報漏えい抑止
  - 既存実績ある暗号ライブラリ採用（OpenSSL/libsodium等）
