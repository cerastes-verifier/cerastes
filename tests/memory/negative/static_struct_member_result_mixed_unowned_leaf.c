// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/static_struct_member_result_mixed.c
// Pins: 0054 item 7: the same shape returning the long leaf of a mixed-type struct the function does not hold: not covered, nothing to give
// Pre-registered from record 0054 item 7 (S39 e2x): permission at the return
#include <cerastes.h>

struct M {
  int a;
  long b;
  char c;
};

static struct M gm;

long *where_long(void)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  return &gm.b;  // <- here
}
