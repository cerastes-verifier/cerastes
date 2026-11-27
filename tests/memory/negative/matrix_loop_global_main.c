// expect: error check @ L26
// expect: error check @ L34
// mode: c23, c++20
// twin: tests/memory/matrix_loop_global.c
// Pins: 0064 items 9 and 10 (Part E slice 3a; audit round E2a, finding 3): the must-fail twin of matrix_loop_global.c for the reach of a loop over a GLOBAL matrix in `main`, where a loop that does not state a value keeps none of it: two global matrices, one loop each, one `check` after each loop that fails: a store by name (`g1[i][1] = 0`, then `g1[0][0] == 1`) and an owning callee of the whole global (`wg2()` under `owns(g2)`); the auditor's l11 and l11c. RESTRUCTURED under question 387 (a `main` with four loops over four globals took 269 s in the harness: the cost is the number of loops in one function): the other two loops (an owning callee over a row, a store through a pointer to a row) are in negative/matrix_loop_global_main_rows.c, and the loop-free control that verifies is tests/memory/matrix_loop_global_main_control.c
// Pre-registered (test-mutator-10, Task 1/2, before any run): check at the line of each claim after a loop: the first two rows of the file's earlier form (their rule is unchanged), renumbered by the restructure; corrected values: none (the claims are as the auditor measured them)
#include <cerastes.h>

static int g1[2][3] = {{1, 2, 3}, {4, 5, 6}};
static int g2[2][3] = {{1, 2, 3}, {4, 5, 6}};

void wg2(void)
    CERASTES_REQUIRES(cerastes_owns(g2))
    CERASTES_ENSURES(cerastes_owns(g2));

int main(void)
{
  int s = (g1[0][0] + g2[0][0]) * 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    g1[i][1] = 0;
    i = i + 1;
  }
  CERASTES_CHECK(g1[0][0] == 1);  // <- here
  i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    wg2();
    i = i + 1;
  }
  CERASTES_CHECK(g2[0][0] == 1);  // <- here
  return s;
}
