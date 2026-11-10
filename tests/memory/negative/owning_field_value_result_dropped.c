// expect: error leak @ L30
// mode: c23, c++20
// twin: tests/memory/owning_field_value_result.c
// Pins: 0055 item 6: the caller takes the result's block into b and never frees it: reported at the return, where b's lifetime ends
// Pre-registered from record 0055 item 6 (copies and values): leak at the return statement (lifetime-end row of b); frozen base: leak at the closing brace of use
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf make(int v)
    CERASTES_ENSURES(r, r.n == v && (r.data == NULL || cerastes_owns_block(r.data)))
{
  struct buf s = { NULL, v };
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = v;
  }
  return s;
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct buf b = make(3);
  int n = b.n;
  return n;  // <- here
}
