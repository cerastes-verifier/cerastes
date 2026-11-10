// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/not_invalidating_guard_equivalent.c
// Pins: 0058 item 2 ("taken implies returned on every path"): the requires takes *p whatever c is and the ensures returns it only where c != 0, so on the path c == 0 the contract takes *p without returning it, which the marker forbids: the new f__wf row fails (permission at the ensures clause, a best reading of the line, as owning_field_bodiless_untied.c)
// Pre-registered from record 0058 item 2: one permission error at the ensures clause (the line is a best reading: confirm on the first run, question 160); the unmarked contract verifies today (build/m4.5d2b2d/emul/g2_today.c)
#include <cerastes.h>

void maybe(int *p CERASTES_NOT_INVALIDATING, int c)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(c != 0 ? cerastes_owns(*p) : 1);  // <- here

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 1;
  maybe(&x, 1);
  return 0;
}
