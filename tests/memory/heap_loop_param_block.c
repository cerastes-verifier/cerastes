// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10: a parameter that owns its block (owns_block in both lists): the loop writes its members and the block is returned to the caller
#include <cerastes.h>
#include <stdlib.h>

struct acc {
  int bal;
  int n;
};

void bump(struct acc *a, int n)
    CERASTES_REQUIRES(cerastes_owns_block(a))
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns_block(a))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    a->bal = i;
    a->n = a->bal;
    i += 1;
  }
}

int use(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct acc *a = (struct acc *)calloc(1, sizeof(struct acc));
  if (a == NULL) return -1;
  bump(a, n);
  free(a);
  return 0;
}
