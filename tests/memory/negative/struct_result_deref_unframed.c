// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/struct_result_deref.c
// Pins: 0045 item 9: *r.q read with nothing saying r.q is p: permission at the clause
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

struct P mk(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && *r.q == 1)  // <- here
{
  *p = 1;
  struct P s = {p};
  return s;
}
