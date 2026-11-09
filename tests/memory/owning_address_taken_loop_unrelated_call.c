// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 6 and audit round 2 (Parts C and D), probe b4: an address-taken owner freed through its alias and cleared before a loop that calls a function naming nothing of it: the call must not make held_v unknown, and v's exit row finds nothing held
// Pre-registered from record 0054 item 6 and audit probe b4 (a leak at the return was a false alarm when written): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int sq(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x * x)
{
  return x * x;
}

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  int **q = &v;
  free(*q);
  *q = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    (void)sq(i);
    i += 1;
  }
  return 0;
}

int main(void)
{
  return f();
}
