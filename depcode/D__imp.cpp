#include "D__imp.h"
#ifdef __MINGW64__

#include <winsock2.h>
#include "stdio.h"

// __mingw_vsnprintf を __imp__vsnprintf のラッパとして公開
extern "C" int __imp__vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
    return __mingw_vsnprintf(str, size, fmt, ap);
}
extern "C" int __imp_accept(int s, struct sockaddr *addr, int *addrlen)
{
    return accept(s, addr, addrlen);
}
#endif
