// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10: a call in the body that borrows the held block (owns_block in both lists) writes it: the place is reached; continue and the body's end carry the block
#include <cerastes.h>
#include <stdlib.h>

struct acc {
  int bal;
};

void bump(struct acc *a)
    CERASTES_REQUIRES(cerastes_owns_block(a))
    CERASTES_ENSURES(cerastes_owns_block(a))
{
  a->bal = 1;
}

int run(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct acc *a = (struct acc *)calloc(1, sizeof(struct acc));
  if (a == NULL) {
    return -1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    bump(a);
    if (i == 50) {
      i += 1;
      continue;
    }
    i += 1;
  }
  free(a);
  return 0;
}
