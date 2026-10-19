#include <cerastes.h>

static inline int one_h(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == 1)
{
  return x;
}
