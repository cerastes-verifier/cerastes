// expect: verified
// expect: native-ok
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// Pins: 0040 item 3 (0037 item 2): owning is the owns conjunct restrict needs; no other contract is written
#include <cerastes.h>
#include <stddef.h>

int get(CERASTES_OWNING int *restrict p)
    CERASTES_ENSURES(r, p == NULL ? r == 0 : r == *p)
{
  if (p == NULL) {
    return 0;
  }
  return *p;
}
