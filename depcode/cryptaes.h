#pragma once
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

#ifndef SHUT_RC
#define SHUT_RC 0
#endif

#ifndef SHUT_SD
#define SHUT_SD 1
#endif

#ifndef SHUT_RW
#define SHUT_RW 2
#endif

namespace unet::cry
{
    int send_crypt(int s, const char *buf, int len, int flags);
    int recv_crypt(int s, char *buf, int len, int flags);
    int accept_crypt(int s, struct sockaddr *addr, int *addrlen);
    int connect_crypt(int s, const struct sockaddr *name, int namelen);
    int close_crypt(int s);
    int shutdown_crypt(int s, int how);
}
