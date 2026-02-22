# cppnet Documentation

[Japanese version](README.md)

## Overview

`cppnet` is a lightweight C++ networking library for TCP/UDP/SSL communication.  
Using `#include <unet.h>` pulls in the main public headers:

- `netdefs.h`
- `base.h`
- `server.h`
- `client.h`
- `udp.h`
- `http.h`

## Public API (Core)

### Initialization and Utilities

- `unet::netcpp_start()`
- `unet::netcpp_stop()`
- `unet::getipaddr(...)`
- `unet::getipaddrinfo(...)`
- `unet::ip2str(...)`

### Socket Types

- `unet::TCP_c`
- `unet::SSL_c`
- `unet::UDP_c`

### Main Classes

- `unet::Server` (`typedef Server_com` is available)
- `unet::Client`
- `unet::Standby`
- `unet::net_core` (used in server callbacks)
- `unet::udp_core`
- `unet::net_base` (shared base)

### HTTP Helpers

- `unet::http::get_http_request_header(...)`
- `unet::http::get_http_result_header(...)`
- `unet::http::extract_http_header(...)`
- `unet::http::extract_http_body(...)`

## Verified Scenarios (`test2.cpp`)

`test2.cpp` covers these 11 scenarios:

- HTTP helper generation/parsing
- IP helper functions
- `net_base` surface behavior
- `net_base::recv_data` overloads
- `Server` + `Client` (TCP)
- `Server` + `Client` (SSL)
- `Standby` client mode (TCP)
- `Standby` client mode (SSL)
- `Standby` server mode (TCP)
- `Standby` server mode (SSL)
- `udp_core` send/receive, timeout, and address-aware receive

## Build

### Example (Linux / Ubuntu)

```bash
sudo apt install -y g++ clang make libssl-dev zlib1g-dev
```

```bash
git clone https://github.com/GFDSA030/cppnet.git cppnet
cd cppnet
make
```

Adjust `IFILE` / `LFILE` in `Makefile` for your environment (especially on Windows).

## Run

```bash
./main.out
./test.out
./test2.out
```

## Minimal Examples

### Server + Client (TCP)

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

### UDP (`udp_core`)

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

## Notes

- SSL mode requires certificate/private key files (for example: `server.crt`, `server.key`).
- `test2.cpp` uses IPv6 loopback (`::1`), so IPv6 must be enabled in your environment.

## License

[zlib License](LICENSE)
