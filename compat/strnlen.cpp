Copyright (c) 2017-2022 The Lex Core developers
# Distributed under the MIT software license, see the accompanying

#if defined(HAVE_CONFIG_H)
#include "config/lexcoin-config.h"
#endif

#include <cstring>

#if HAVE_DECL_STRNLEN == 0
size_t strnlen( const char *start, size_t max_len)
{
    const char *end = (const char *)memchr(start, '\0', max_len);

    return end ? (size_t)(end - start) : max_len;
}
#endif // HAVE_DECL_STRNLEN
