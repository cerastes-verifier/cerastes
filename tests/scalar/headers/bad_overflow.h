#include <cerastes.h>

static inline int twice_h(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r >= x)
{
  return x + x;
}
