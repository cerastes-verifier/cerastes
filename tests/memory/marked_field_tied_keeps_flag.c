// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit B1): a tied borrowed site keeps the flags: the caller wrote the buffer, the callee only writes n and ties p->data == old(p->data), and the caller reads the buffer after the call
// Pre-registered from record 0058 item 4 (audit B1, the il-analyst's rows): verifies in both modes (partF2/f1_tied_untouched.c)
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
void reset(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->data == cerastes_old(p->data))
{
  p->n = 0;
}
int use(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = 5;
  }
  reset(&s);
  if (s.data != NULL) {
    int y = *s.data;
    s.n = y;
  }
  free(s.data);
  return 0;
}
