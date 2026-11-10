// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 7, 0055 items 3, 5 (S6): a function fills the owning leaf of a static struct local once and returns &s under ensures(r, owns(*r)): the result's closure (r->data == NULL || owns_block(r->data)) is covered by the implicit owns(s), and the caller writes through it
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes (new: no base form; static_struct_member_result.c is the owns(*r) precedent without an owning field)
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
  return &s;
}

int main(void)
{
  struct buf *p = where();
  p->n = 2;
  return 0;
}
