// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/struct_result_deref.c
// Pins: 0045 item 1 (question 106): owns(*r.q), an atom through a result member with no requires partner: M4.5d2b
#include <cerastes.h>
struct P {
  int *q;
};

struct P f(int *p)
    CERASTES_ENSURES(r, cerastes_owns(*r.q))  // <- here
{
  struct P w = {p};
  return w;
}
