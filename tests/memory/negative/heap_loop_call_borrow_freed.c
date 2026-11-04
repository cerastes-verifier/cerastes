// expect: error permission @ L38
// mode: c23, c++20
// twin: tests/memory/heap_loop_call_borrow.c
// Pins: 0047 item 10: the body frees the block that the iteration holds on the path that does not continue: the back edge at the body's end lacks its token (the continue path still carries it)
// Pre-registered from the loops branch's positive (record 0047 item 10), written after it: the expectation was verified once on the single file before it was written
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
    free(a);
  }  // <- here
  free(a);
  return 0;
}
