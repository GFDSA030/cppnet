#pragma once
#include "stdint.h"
#ifdef __MINGW64__
// __mingw_vsnprintf を __imp__vsnprintf のラッパとして公開
extern "C" int __imp__vsnprintf(char *str, size_t size, const char *fmt, va_list ap);
extern "C" int __imp_accept(int s, struct sockaddr *addr, int *addrlen);
#endif
