// Library fixture: `#pragma GCC system_header` in a library file is a no-op (decision 0046 item 3, amended): the file is
// already a --sys_include file for EDG, so the pragma starts no new record, and cerastes decides by path, so what
// follows it is still a library file (see lib_pragma_bad.h).
#ifndef LIB_PRAGMA_H
#define LIB_PRAGMA_H
#include <cerastes.h>
#pragma GCC system_header

static inline int lib_after_pragma(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

#endif
