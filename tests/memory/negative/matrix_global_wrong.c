// expect: error postcondition @ L21
// expect: error check @ L26
// mode: c23, c++20
// twin: tests/memory/matrix_global.c
// Pins: 0064 item 9 and the coordinator's ruling on question 358 (i): in its own function each, the static local matrix's implicit atom (its entry value is unknown, so the result may be 100, and the postcondition r <= 99 fails at the return) and the global matrix's listed value (g[1][2] is 6, not 5: check)
// Pre-registered from record 0064 item 9 (Part E), before any run: one error in each function: postcondition at next's return, check in main
#include <cerastes.h>

static int g[2][3] = {{1, 2, 3}, {4, 5, 6}};

int next(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, 1 <= r && r <= 99)
{
  static int c[2][3] = {{0}};
  int v = c[i][j];
  if (v < 0 || v > 99) {
    v = 0;
  }
  c[i][j] = v + 1;
  return c[i][j];  // <- here
}

int main(void)
{
  CERASTES_CHECK(g[1][2] == 5);  // <- here
  return g[0][0] - 1;
}
