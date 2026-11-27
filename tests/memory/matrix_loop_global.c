// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 9 and 10 (Part E slice 3a; audit round E2a, finding 3): loops over a GLOBAL matrix and a `static` local matrix. In `main` (the one function that holds every global at its initial value) a loop that reads `g` keeps every value (its invariant reads `g[1][2]` and `g[i][0]`) while the same loop writes a local matrix beside it; a function under `owns(g)` writes the global in a loop and keeps `g[0][0] == 1` by its invariant; a `static` local matrix read in a loop keeps its value. Twins: negative/matrix_loop_global_main.c (a store by name, an owning callee, an owning callee over a row, a store through a pointer: each loses the values), negative/matrix_loop_global_held.c (a loop that loses the held value, a write under `reads(g)`, a read outside the matrix, a read with no atom, a `static` local's store); the auditor's l11b, l11g, l12b and l13b
// Pre-registered from record 0064 items 5, 9 and 10 (Part E; audit round E2a, finding 3), before any run: verifies
#include <cerastes.h>

static int g[2][3] = {{1, 2, 3}, {4, 5, 6}};

int held(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[0][0] == 1)
    CERASTES_ENSURES(r, cerastes_owns(g) && g[0][0] == 1)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && g[0][0] == 1)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    g[i][1] = 0;
    i = i + 1;
  }
  return 0;
}

int static_local(void)
{
  static int c[2][3] = {{1, 2, 3}, {4, 5, 6}};
  c[0][0] = 1;
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    s = c[i][1] > 0 ? 1 : 0;
    i = i + 1;
  }
  CERASTES_CHECK(c[0][0] == 1);
  return s;
}

int main(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && g[1][2] == 6 && (i == 2 || g[i][0] > 0))
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    s = g[i][2];
    m[i][1] = 0;
    i = i + 1;
  }
  CERASTES_CHECK(g[0][0] == 1 && g[1][2] == 6);
  return m[0][2] * 0 + s * 0;
}
