// expect: error precondition @ L29
// expect: error postcondition @ L36
// mode: c23, c++20
// twin: tests/memory/matrix_value_elems.c
// Pins: 0064 item 12 (question 353, the second type of the "tested so" pair): two failures, one per function. shifted: the caller's cells are shifted by one against the callee's requires (every cell of v and k hold the value one less: k is 6 where the callee needs 7), so the precondition fails at the call; make_wrong: the RESULT's k is 6 where the ensures says 7: the postcondition fails at the return; each has its right-value sibling in tests/memory/matrix_value_elems.c (use and make)
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): precondition at the call, postcondition at the return, once each
#include <cerastes.h>

struct W {
  int a[2];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

int pick(struct T t)
    CERASTES_REQUIRES(t.k == 7)
    CERASTES_ENSURES(r, r == 7)
{
  return t.k;
}

int shifted(void)
{
  struct T t = {{{{0, 1}, 2}, {{3, 4}, 5}}, 6};
  return pick(t);  // <- here
}

struct T make_wrong(void)
    CERASTES_ENSURES(r, r.k == 7)
{
  struct T t = {{{{0, 1}, 2}, {{3, 4}, 5}}, 6};
  return t;  // <- here
}
