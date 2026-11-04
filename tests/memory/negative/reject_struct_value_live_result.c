// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/struct_value_result.c
// Pins: 0044 item 4, 0045 item 1 (question 106): live(r.q) of a pointer member of the result: rejected until M4.5d2b (with owns(*r.q) and every ensures atom without a requires partner)
#include <cerastes.h>

struct Q {
  int *q;
};

struct Q f(int *p)
    CERASTES_ENSURES(r, cerastes_live(r.q))  // <- here
{
  struct Q w = {p};
  return w;
}
