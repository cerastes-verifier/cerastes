// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit B1): a callee that frees the marked owning field's buffer and stores a fresh one, with no tie: the untied borrowed site claims no flags (a freed address may come back with unwritten contents), so the callee verifies, and the caller (which frees the field) too; before B1 this shape exited 3
// Pre-registered from record 0058 item 4 (audit B1, the il-analyst's rows): verifies in both modes (build/m4.5d2b2d/partF2/f2_untied_replaces.c)
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
  reset(&s);
  free(s.data);
  return 0;
}
