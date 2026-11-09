// expect: error leak @ L21
// mode: c23, c++20
// twin: tests/memory/owning_address_taken_loop_unrelated_call.c
// Pins: 0054 item 6 ("Stores through an alias") and audit round 2 (Parts C and D), probe b4: the same program without the free: the store through the alias clears the owner while it holds a block
// Pre-registered from record 0054 item 6: leak at the store through the alias
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
  *q = NULL;  // <- here
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
