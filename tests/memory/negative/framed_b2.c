// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/framed_guarded.c
// Pins: 0037 item 7 (audit B2): the post-state read of *q is framed only by owns(*p), which may be a different cell; a declaration, so the body gives no second error
#include <cerastes.h>
#include <stddef.h>

void b2(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
    CERASTES_ENSURES(*q == 5);

void use_b2(void)
{
  int x = 0;
  int y = 0;
  b2(&x, &y);
}
