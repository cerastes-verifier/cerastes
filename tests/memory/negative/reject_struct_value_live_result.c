// expect: error lifetime @ L15
// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/struct_value_result.c
// Pins: 0044 item 4, 0045 item 1 (question 106), flipped by record 0052 item 2 and owner question 161 (M4.5d2b1): live(r.q) of a result member is accepted now; this callee has only the unframed pointer p, so the initializer {p} is a lifetime error (0028 item 3a) and the return gives r.q without its liveness (permission): both are the expectation
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
