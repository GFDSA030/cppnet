#pragma once
#include "netdefs.h"

namespace unet::cry
{
    int send_crypt(int s, const char *buf, int len, int flags);
    int recv_crypt(int s, char *buf, int len, int flags);
    int accept_crypt(int s, struct sockaddr *addr, int *addrlen);
    int connect_crypt(int s, const struct sockaddr *name, int namelen);
    int close_crypt(int s);
    int shutdown_crypt(int s, int how);
}
