#ifndef NETDEFS
#define NETDEFS
// define here

#define NETCPP_SSL_AVAILABLE

#define NETCPP_BLOCKING

//

#if defined(__MINGW32__) // OS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <unistd.h>

#define ioctl ioctlsocket

#elif defined(_MSC_VER) // OS

#include <winsock2.h>
#include <ws2tcpip.h>

#define ioctl ioctlsocket

#pragma comment(lib, "ws2_32.lib")

#elif defined(__linux__) // OS

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#elif defined(__APPLE__) // OS

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#endif // OS

#ifdef NETCPP_SSL_AVAILABLE

#include <openssl/err.h>
#include <openssl/ssl.h>
namespace unet
{
    constexpr bool __SSL = 0;
}
#define NETCPP_sslfnc(fnc, ...) fnc(__VA_ARGS__);

#else // NETCPP_SSL_AVAILABLE

#pragma message("SSL is disabled")
namespace unet
{
    constexpr bool __SSL = 1;
    typedef char SSL;
    typedef char SSL_CTX;
}
#define NETCPP_sslfnc(fnc, ...) 0;

#endif // NETCPP_SSL_AVAILABLE

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <stdio.h>

static volatile unsigned long long testno = 0;

//[[deprecated("DEBUG_PRINT is used! This will print debug information.")]]
#define DEBUG_PRINT printf("file:%s  line:%d  no:%llu  \n", __FILE__, __LINE__, testno++);

namespace unet
{

    enum sock_type : int
    {
        unknown = -1,
        TCP_c = 80,
        SSL_c = 443,
        UDP_c = 100
    };

    enum status
    {
        error = -1,
        success = 0,
        warning = 1,
        online = 4,
        offline = 5
    };

} // namespace unet

#endif // NETDEFS
