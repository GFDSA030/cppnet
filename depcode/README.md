# frag モジュール README

`frag` は小粒なユーティリティ群です。公開APIは主に `fasm` 名前空間配下（inline namespace 利用）で提供されます。

## ヘッダ一覧

- `D_aes.h`
  - namespace: `fasm::crypt`
  - class: `AES128`, `AES256`
  - API: `encrypt`, `decrypt`

- `D_rsa.h` (`boost/multiprecision/cpp_int.hpp` がある場合のみ有効)
  - namespace: `fasm::crypt`
  - class: `RSA`
  - API: `generate_keys`, `encrypt`, `decrypt`, `export_*_pem`, `import_*_pem`

- `D_ecdsa.h` (`boost/multiprecision/cpp_int.hpp` がある場合のみ有効)
  - namespace: `fasm::crypt`
  - class: `ECDSA`
  - API: `generate_keys`, `sign`, `verify`, `export_*_pem`, `import_*_pem`

- `D_cryptaes.h`
  - namespace: `fasm::net`（`unet::cry` エイリアスあり）
  - API: `send_crypt`, `recv_crypt`, `accept_crypt`, `connect_crypt`, `close_crypt`, `shutdown_crypt`

- `D_sha.h`
  - namespace: `fasm::hash`
  - class: `SHA1`, `SHA256`, `SHA512`
  - API:
    - `SHA1::hash`, `SHA1::str`
    - `SHA256::hash256`, `SHA256::str256`, `SHA256::hash224`, `SHA256::str224`
    - `SHA512::hash512`, `SHA512::str512`, `SHA512::hash384`, `SHA512::str384`

- `D_md5.h`
  - namespace: `fasm::hash`
  - class: `MD5`
  - API: `md5_hex`, `hash`, `str`

- `D_md5c.h`
  - namespace: `fasm::hash::md5`
  - API: `MD5Init`, `MD5Update`, `MD5Final`（Cスタイル実装）

- `D_baseN.h`
  - namespace: `fasm::enc`
  - API: `base16/base32/base64` の encode/decode と汎用 `baseN_encode/decode`
  - table定数: `base16_table`, `base32_table`, `base58_table`, `base64_table`, `base85_table`, `base91_table`

- `D_conv.h`
  - namespace: `fasm::util`
  - API: `str2vec`, `vec2str`

- `D_complession.h` (`zlib.h` がある場合のみ有効)
  - namespace: `fasm::data`
  - API: `complession`, `decomplession`

- `D_uuid.h`
  - namespace: `fasm::id`
  - class: `uuid`, `uuidshort`
  - API: `uuid::GenV1/GenV4/GenV7`, `str`, `encode`, `decode`

- `D_cpuid.h`
  - namespace: `fasm::device`
  - struct: `CPUInfo`, `SMBIOSInfo`
  - API: `get_cpu_info`, `get_smbios`, `generate_device_fingerprint`

- `D_solveWeighted.h`
  - namespace: `fasm::math`
  - API: `polyFit`, `robustFit`

- `D_wchar-char.h`
  - namespace: `fasm::util`
  - API: `narrow`, `widen`

- `D_getch.h`
  - namespace: `fasm::in`
  - API: `D_getch`, `D_kbhit`

- `D_guipanel.h` (`SDL3/SDL.h` がある場合のみ include)
  - SDL3 ヘッダラッパ

- `D__imp.h`
  - MinGW向けの `__imp_*` 補助宣言

- `_all.h`
  - 上記ヘッダ一式をまとめて include する集約ヘッダ

## 依存関係

- 必須
  - C++17 以降

- 条件付き
  - Boost.Multiprecision (`D_rsa.h`, `D_ecdsa.h`)
  - zlib (`D_complession.h`)
  - SDL3 (`D_guipanel.h`)

- OS/環境依存
  - `D_cryptaes.h`, `D_cpuid.h`, `D_getch.h` はプラットフォーム差分あり

## 使い方

個別に使う場合:

```cpp
#include "frag/D_aes.h"
#include "frag/D_conv.h"
```

まとめて使う場合:

```cpp
#include "frag/_all.h"
```

## 小サンプルコード

AES + `str2vec` / `vec2str`:

```cpp
#include "frag/D_aes.h"
#include "frag/D_conv.h"

std::vector<uint8_t> key(32, 0x11);
fasm::crypt::AES256 aes(key);
auto enc = aes.encrypt(fasm::util::str2vec("hello"));
auto dec = fasm::util::vec2str(aes.decrypt(enc));
```

SHA-256 / MD5:

```cpp
#include "frag/D_sha.h"
#include "frag/D_md5.h"

auto sha256_hex = fasm::hash::SHA256::str256("hello");
auto md5_hex = fasm::hash::MD5::str("hello");
```

Base64:

```cpp
#include "frag/D_baseN.h"

auto b64 = fasm::enc::base64_encode("hello");
auto raw = fasm::enc::base64_decode(b64);
```

UUID:

```cpp
#include "frag/D_uuid.h"

auto id = fasm::id::uuid::GenV7();
auto s = id.str();
auto short_id = id.encode().str();
```

`wstring` / `string` 変換:

```cpp
#include "frag/D_wchar-char.h"

std::wstring ws = L"hello";
std::string s;
fasm::util::narrow(ws, s);
```

多項式近似:

```cpp
#include "frag/D_solveWeighted.h"

std::vector<std::pair<double, double>> pts = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 7.0}};
auto coef = fasm::math::polyFit(pts, 2);
```

デバイス情報:

```cpp
#include "frag/D_cpuid.h"

auto cpu = fasm::device::get_cpu_info();
auto fp = fasm::device::generate_device_fingerprint();
```

条件付きヘッダの利用例:

```cpp
#if __has_include("boost/multiprecision/cpp_int.hpp")
#include "frag/D_rsa.h"
#endif

#if __has_include("zlib.h")
#include "frag/D_complession.h"
#endif
```

## 注意

- 一部ヘッダは `__has_include(...)` 条件で有効/無効が切り替わります。
- 名前空間はヘッダ上 `fasm::inline ...` で定義されていますが、利用時は `fasm::<name>` で参照できます。
