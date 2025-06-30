# cppserver_linux Documentation (English)

[Japanese version](README.md)

## Overview

cppserver_linux is a cross-platform C++ network library for easily building TCP/UDP/SSL servers and clients.

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
git clone https://github.com/GFDSA030/cppserver_linux.git cpp-net
cd cpp-net
make
```

## Classes & Constants

### Communication Types

- `unet::TCP_c` : TCP communication
- `unet::UDP_c` : UDP communication (untested)
- `unet::SSL_c` : SSL communication (OpenSSL required)

### Main Classes

- `unet::net_base` : Base class for server/client (used as a base class)
- `unet::ServerTCP` : TCP server class
- `unet::ServerSSL` : SSL server class
- `unet::Server_com` : Multi-type server class (with overhead)
- `unet::ClientTCP` : TCP client class
- `unet::ClientSSL` : SSL client class
- `unet::Client_com` : Multi-type client class (with overhead)

## Class Examples

### unet::net_base (Base Class)

```cpp
#include <unet.h>
#include <iostream>
int main() {
    unet::netinit();
    unet::net_base base;
    // Basic socket operations
    base.close_s();
    unet::netquit();
}
```

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

## License

[zlib License](LICENSE)
