# cppnet Documentation (English)

[Japanese version](README.md)

## Overview

cppnet is a cross-platform C++ network library for easily building TCP/UDP/SSL servers and clients.

## Features

- Supports Windows/Linux/macOS
- TCP/UDP/SSL communication
- Simple API design
- SSL communication via OpenSSL
- Both server and client implementations

## Supported Environments

- Windows (MinGW, MSVC)
- Ubuntu (gcc, clang)
- macOS (untested)

## Build Environment Setup

```bash
apt install -y clang gcc libssl-dev make
```

## Build Instructions

```bash
git clone https://github.com/GFDSA030/cppnet.git cppnet
```

You need to set the OpenSSL path in the Makefile. Set the header path to `IFILE` and the library path to `LFILE` as appropriate for your environment.

```bash
cd cppnet
make
```

## Classes & Constants

### Communication Types

- `unet::TCP_c` : TCP communication
- `unet::UDP_c` : UDP communication (untested)
- `unet::SSL_c` : SSL communication (OpenSSL required)

### Main Classes

- `unet::net_core` : common communication class for server
- `unet::ServerTCP` : TCP server class
- `unet::ServerSSL` : SSL server class
- `unet::Server_com` : Multi-type server class (with overhead)
- `unet::ClientTCP` : TCP client class
- `unet::ClientSSL` : SSL client class
- `unet::Client_com` : Multi-type client class (with overhead)

## Class Examples

### unet::ServerTCP (TCP Server)

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

### unet::ServerSSL (SSL Server)

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

### unet::Server_com (Multi-type Server)

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

### unet::ClientTCP (TCP Client)

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

### unet::ClientSSL (SSL Client)

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

### unet::Client_com (Multi-type Client)

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

### unet::ServerUDP (UDP Server)

```cpp
#include <server.h>
#include <iostream>
int main() {
    unet::netinit();
    unet::ServerUDP svr(8000);
    char buf[1024];
    int len = svr.recv_data(buf, sizeof(buf));
    std::cout << "Received: " << std::string(buf, len) << std::endl;
    svr.send_data("Hello UDP!", 10);
    svr.close_s();
    unet::netquit();
}
```

### unet::ClientUDP (UDP Client)

```cpp
#include <client.h>
#include <iostream>
int main() {
    unet::netinit();
    unet::ClientUDP cli("localhost", 8000);
    cli.send_data("Hello UDP server", 17);
    char buf[1024];
    int len = cli.recv_data(buf, sizeof(buf));
    std::cout << std::string(buf, len) << std::endl;
    cli.close_s();
    unet::netquit();
}
```

## License

[zlib License](LICENSE)
