// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 9 and 10 (question 387): the loop-free control of negative/matrix_loop_global_main.c and negative/matrix_loop_global_main_rows.c: the same four global matrices and the same four claims in `main`, with no loop, verify (program_start gives every matrix its initial values), so the twins' `check` failures come from the loops, not from the claims
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the same four claims as the twins' four rows; instance 8's first split of the file was void: its checks failed with no loop at all)
#include <cerastes.h>

static int g1[2][3] = {{1, 2, 3}, {4, 5, 6}};
static int g2[2][3] = {{1, 2, 3}, {4, 5, 6}};
static int g3[2][3] = {{1, 2, 3}, {4, 5, 6}};
static int g4[2][3] = {{1, 2, 3}, {4, 5, 6}};

int main(void)
{
  CERASTES_CHECK(g1[0][0] == 1 && g2[0][0] == 1 && g3[0][1] == 2 && g4[0][1] == 2);
  return (g1[0][0] + g2[0][0] + g3[0][0] + g4[0][0]) * 0;
}
