// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/owning_field_static_return.c
// Pins: 0055 item 3 (S6): the function frees the block of s.data without setting it to NULL and returns &s under owns(*r): the result's closure clause has no block to take, "returns without owning the whole allocation `s.data` points to"
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), permission at the return row (the return statement; new: no base form)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *where(void)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  static struct buf s;
  if (s.data == NULL) {
    s.data = (int *)malloc(sizeof(int));
    if (s.data != NULL) {
      *s.data = 1;
    }
  }
  free(s.data);
  return &s;  // <- here
}

int main(void)
{
  struct buf *p = where();
  p->n = 2;
  return 0;
}
