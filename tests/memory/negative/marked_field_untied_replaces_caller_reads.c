// expect: error uninitialized @ L25
// mode: c23, c++20
// twin: tests/memory/marked_field_untied_replaces.c
// Pins: 0058 item 4 (audit B1): after an untied replacing callee the caller does not know the field's buffer holds a value, although it wrote one before the call: reading it is uninitialized (the sound direction: an untied marked site claims no flags)
// Pre-registered from record 0058 item 4 (audit B1, the il-analyst's rows): one uninitialized error at the caller's read (partF2/f5_untied_replaces_caller_reads.c)
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
void reset(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  p->data = (int *)malloc(sizeof(int));
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
    int y = *s.data;  // <- here
    s.n = y;
  }
  free(s.data);
  return 0;
}
