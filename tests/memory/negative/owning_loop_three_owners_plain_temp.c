// expect: error leak @ L35
// mode: c23, c++20
// twin: tests/memory/owning_loop_three_owners.c
// Pins: 0054 item 3: the same rotation through a plain temporary: a = b overwrites an owner whose old block is held only by t (as owning_loop_swap_plain_temp.c)
// Pre-registered from record 0054 item 3 and 0052 item 3: leak at a = b
#include <cerastes.h>
#include <stdlib.h>

int rotate(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return -1;
  }
  CERASTES_OWNING int *b = (int *)malloc(sizeof(int));
  if (b == NULL) {
    free(a);
    return -1;
  }
  CERASTES_OWNING int *c = (int *)malloc(sizeof(int));
  if (c == NULL) {
    free(a);
    free(b);
    return -1;
  }
  *a = 0;
  *b = 1;
  *c = 2;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int *t = a;
    a = b;  // <- here
    b = c;
    c = t;
    i += 1;
  }
  free(a);
  free(b);
  free(c);
  return 0;
}
