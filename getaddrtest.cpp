
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

int main()
{
    char *hostname = "localhost";
    char *service = "http";
    struct addrinfo hints, *res0, *res;
    int err;
    SOCKET sock;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = PF_UNSPEC;

    WSADATA data;
    WSAStartup(MAKEWORD(2, 0), &data);

    if ((err = getaddrinfo(hostname, service, &hints, &res0)) != 0)
    {
        printf("error %d\n", err);
        return 1;
    }

    for (res = res0; res != NULL; res = res->ai_next)
    {
        sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock == INVALID_SOCKET)
        {
            continue;
        }

        if (!connect(sock, res->ai_addr, res->ai_addrlen))
        {
            closesocket(sock);
            continue;
        }

        break;
    }

    freeaddrinfo(res0);

    // sockが有効なら通信するコードを書く。。。

    WSACleanup();

    return 0;
}