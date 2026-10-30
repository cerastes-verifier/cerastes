// expect: error unsequenced @ L20
// mode: c23, c++20
// twin: tests/memory/two_memory_calls.c
// Pins: 0039 item 4: bump(&x) + bump(&x): both calls own *x and C does not order them, so arg1 != arg2 fails; reported at the later call
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int twice(void)
    CERASTES_ENSURES(r, r >= 0 || r < 0)
{
  int x = 1;
  return bump(&x) + bump(&x);
}
