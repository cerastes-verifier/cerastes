// expect: error lifetime @ L14
// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/struct_result_deref.c
// Pins: 0045 item 1 (question 106), flipped by record 0052 item 2 and owner question 161 (M4.5d2b1): owns(*r.q) of a result member is accepted now; this callee has only the unframed pointer p, so the initializer {p} is a lifetime error (a use of an unframed pointer, 0028 item 3a) and the return gives *r.q without holding it (permission): both are the expectation
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
