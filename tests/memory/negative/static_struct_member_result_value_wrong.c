// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/static_struct_member_result_value.c
// Pins: 0054 item 7: the read after a covered atom is a real obligation of the callee (*r is 2, not 1)
// Pre-registered from record 0054 item 7: postcondition at the return
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int *where(void)
    CERASTES_ENSURES(r, cerastes_owns(*r) && *r == 1)
{
  static struct S s;
  s.x = 2;
  return &s.x;  // <- here
}
