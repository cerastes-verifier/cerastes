// Library fixture: the twin of lib_body.h with a body that breaks its contract.
#ifndef LIB_BODY_BAD_H
#define LIB_BODY_BAD_H
#include <cerastes.h>

static inline int lib_bad_one(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == 1)
{
  return x;
}

#endif
