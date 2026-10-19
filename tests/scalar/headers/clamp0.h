#include <cerastes.h>

static inline int clamp0(int x)
    CERASTES_REQUIRES(x >= -1000 && x <= 1000)
    CERASTES_ENSURES(r, r >= 0 && r <= 1000)
{
  return x < 0 ? 0 : x;
}
