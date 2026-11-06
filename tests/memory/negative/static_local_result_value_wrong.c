// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/static_local_result_value.c
// Pins: 0052 item 2 (audit B5): the function leaves a negative value in the static it returns: the clause *r >= 0 fails at the return
// Pre-registered from record 0052 item 2: postcondition at the return statement
#include <cerastes.h>

int *where(void)
    CERASTES_ENSURES(r, cerastes_owns(*r) && *r >= 0)
{
  static int s = 5;
  s = -1;
  return &s;  // <- here
}
