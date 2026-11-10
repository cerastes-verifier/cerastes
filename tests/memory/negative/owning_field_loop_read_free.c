// expect: error permission @ L27
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_read.c
// Pins: 0055 items 3, 7: after the loop the function frees the field's block and returns without setting p->data to NULL: the closure clause of the ensures has no block to take, "returns without owning the whole allocation p->data points to"
// Pre-registered from record 0055 item 7 (loops): permission at the return row (the closing brace for a void function in S40 (a) a_free_fail.c; here the return statement: confirmed on the frozen base only if the table says so)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int sum(struct buf *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->data != NULL && 0 <= n && n <= 100 && *p->data == 3)
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->data != NULL && r == 3 * n)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == 3 * i && *p->data == 3)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += *p->data;
    i += 1;
  }
  free(p->data);
  return s;  // <- here
}
