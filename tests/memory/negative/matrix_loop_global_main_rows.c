// expect: error check @ L30
// expect: error check @ L39
// mode: c23, c++20
// twin: tests/memory/matrix_loop_global_main_control.c
// Pins: 0064 items 9 and 10 (Part E slice 3a; audit round E2a, finding 3; question 387's second half of negative/matrix_loop_global_main.c): the reach of a loop over a GLOBAL matrix in `main` for a row: an owning callee over a row (`wrow(g3[i], 3)`, then `g3[0][1] == 2`) and a store through a pointer to a row (`int *p = g4[1]`, then `g4[0][1] == 2`) lose the cells they may write; the auditor's l11d and l11e; the loop-free control that verifies is tests/memory/matrix_loop_global_main_control.c
// Pre-registered (test-mutator-10, Task 1/2, before any run): check at the line of each claim after a loop (the last two rows of the earlier form of matrix_loop_global_main.c, their rule unchanged), renumbered by the restructure
#include <cerastes.h>

void wrow(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  p[0] = 0;
  (void)n;
}

static int g3[2][3] = {{1, 2, 3}, {4, 5, 6}};
static int g4[2][3] = {{1, 2, 3}, {4, 5, 6}};

int main(void)
{
  int s = (g3[0][0] + g4[0][0]) * 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    wrow(g3[i], 3);
    i = i + 1;
  }
  CERASTES_CHECK(g3[0][1] == 2);  // <- here
  int *p = g4[1];
  i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    p[i] = 0;
    i = i + 1;
  }
  CERASTES_CHECK(g4[0][1] == 2);  // <- here
  return s;
}
