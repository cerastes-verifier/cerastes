// expect: error uninitialized @ L21
// mode: c23, c++20
// twin: tests/memory/marked_field_tied_keeps_flag.c
// Pins: 0058 item 4 (audit B1): the same tied callee, but the caller never wrote the buffer: the kept row keeps only flags that were set, so reading it after the call is uninitialized
// Pre-registered from record 0058 item 4 (audit B1, the il-analyst's rows): one uninitialized error at the caller's read (partF2/f5b_tied_untouched_caller_reads_unwritten.c)
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
  reset(&s);
  if (s.data != NULL) {
    int y = *s.data;  // <- here
    s.n = y;
  }
  free(s.data);
  return 0;
}
