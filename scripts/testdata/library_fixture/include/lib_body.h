// Library fixture: a body in a library header is verified, and its contract used by callers.
#ifndef LIB_BODY_H
#define LIB_BODY_H
#include <cerastes.h>

static inline int lib_clamp0(int x)
    CERASTES_REQUIRES(x >= -1000 && x <= 1000)
    CERASTES_ENSURES(r, r >= 0 && r <= 1000)
{
  return x < 0 ? 0 : x;
}

#endif
