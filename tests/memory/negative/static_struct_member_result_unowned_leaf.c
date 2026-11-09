// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/static_struct_member_result.c
// Pins: 0054 item 7: a leaf of a struct the function does not hold (a global, not its own static local) is not covered and there is nothing to give: "returns *r without holding it"
// Pre-registered from record 0054 item 7 (S39 e2x, as result_owns_global_not_held.c): permission at the return
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S gs;

int *where(void)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  return &gs.x;  // <- here
}
