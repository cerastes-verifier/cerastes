// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/struct_value_result.c
// Pins: 0044 item 4 (M4.5c2b): live(r.q) of a pointer member of the result
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
