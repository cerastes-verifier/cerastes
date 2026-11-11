// expect: error postcondition @ L20
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/marked_field_strict_global.c
// Pins: 0058 item 4: the default checks stay under --strict-init: the static local's field is set to 2, the ensures says 3
// Pre-registered from record 0058 item 4: one postcondition error at the return
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

int local(void)
    CERASTES_ENSURES(r, r == 3)
{
  static struct pt sl = { NULL, 2 };
  sl.n = 2;
  return sl.n;  // <- here
}
