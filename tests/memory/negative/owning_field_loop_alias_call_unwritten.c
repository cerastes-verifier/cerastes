// expect: error uninitialized @ L34
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_alias_call.c
// Pins: 0056 item 4 (limit 2): the same function with a malloc stored into s.data before the loop and never written: refill requires *p->data to have a value, and the block has none on its first call
// Pre-registered from record 0056 item 4 (not yet run): uninitialized at the call refill(ps) (the message of S41 (0) run 2: calls refill, which needs *p->data to have a value; it may not have one here)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void refill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  p->data = (int *)malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = 1;
  }
}

int f(void)
{
  struct buf s = { NULL, 0 };
  struct buf *ps = &s;
  s.data = (int *)malloc(sizeof(int));
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    refill(ps);
    i = i + 1;
  }
  free(s.data);
  s.data = NULL;
  return 0;
}
