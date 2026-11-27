// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/matrix_value_limit_seven.c
// Pins: 0064 item 12 (question 353): the by-value parameter of exactly 7 counted cells (struct V7 { int m[1][7]; }) is accepted and the function is verified (it is not skipped): a postcondition that the result is 1 where the body returns the cell times 0 fails at the return; its right-value sibling is param7 in tests/memory/matrix_value_limit_seven.c
// Pre-registered from record 0064 item 12 (Part E), before any run (test-mutator-11, 2026-11-27): the one postcondition error at the return
#include <cerastes.h>

struct V7 {
  int m[1][7];
};

int param7_wrong(struct V7 v)
    CERASTES_ENSURES(r, r == 1)
{
  return v.m[0][6] * 0;  // <- here
}
