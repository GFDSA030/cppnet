#pragma once
#include "netdefs.h"

namespace unet::cry
{
    int send_cry(int s, const char *buf, int len, int flags);
    int recv_cry(int s, char *buf, int len, int flags);
    int accept_cry(int s, struct sockaddr *addr, int *addrlen);
    int connect_cry(int s, const struct sockaddr *name, int namelen);
    int close_cry(int _FileHandle);
}
