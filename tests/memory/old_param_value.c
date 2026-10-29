// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 item 5 (pin): cerastes_old(x) of a by-value parameter does not make it address-taken: it stays a Viper local
#include <cerastes.h>
#include <stddef.h>

#include <limits.h>
int succ(int x)
    CERASTES_REQUIRES(x < INT_MAX)
    CERASTES_ENSURES(r, r == cerastes_old(x) + 1)
{
  return x + 1;
}
