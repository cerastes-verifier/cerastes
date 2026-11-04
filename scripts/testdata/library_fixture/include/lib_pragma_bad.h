// Library fixture: the twin of lib_pragma.h whose function after the pragma breaks its contract.  It is still
// verified like user code, so the error is reported here.
#ifndef LIB_PRAGMA_BAD_H
#define LIB_PRAGMA_BAD_H
#include <cerastes.h>
#pragma GCC system_header

static inline int lib_after_pragma_bad(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x;
}

#endif
