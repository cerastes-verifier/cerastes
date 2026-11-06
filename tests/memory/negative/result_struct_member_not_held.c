// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/result_struct_member.c
// Pins: 0052 item 2: owns(*r.q) of the member without holding the cell (the callee has only live(p), so p may be used but its value cell is not held): permission at the return
// Pre-registered from record 0052 item 2: permission at the return statement
#include <cerastes.h>

struct Q {
  int *q;
};

struct Q give_owns(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_owns(*r.q) && r.q == p)
{
  struct Q w = {p};
  return w;  // <- here
}
