// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 6 and audit round 2 (Parts C and D), probe h3: an address-taken owning local in a loop that frees what it holds through the alias and allocates a new block every iteration; v is freed through the alias after the loop
// Pre-registered from record 0054 item 6 and audit probe h3 (verifies today): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING int *v = NULL;
  int **q = &v;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(*q);
    v = (int *)malloc(sizeof(int));
    if (v != NULL) {
      *v = i;
    }
    i += 1;
  }
  free(*q);
  return 0;
}

int main(void)
{
  return f();
}
