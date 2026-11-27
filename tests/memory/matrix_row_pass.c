// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 5 and 8 (Part E): a row m[i] used as a pointer is the row's decay (the bound of i strictly inside, then a pointer to the row's cell 0, a valid int * by the length facts): it is passed to an owns_range(q, 3) callee that writes the three cells and to a reads_range(q, 3) callee, and the cells are read back by name (m[i][2]) and through the callee
// Pre-registered from record 0064 items 5 and 8 (Part E), before any run: verifies
#include <cerastes.h>

void setrow(int *q)
    CERASTES_REQUIRES(cerastes_owns_range(q, 3))
    CERASTES_ENSURES(cerastes_owns_range(q, 3) && q[0] == 1 && q[1] == 2 && q[2] == 3)
{
  q[0] = 1;
  q[1] = 2;
  q[2] = 3;
}

int middle(int *q)
    CERASTES_REQUIRES(cerastes_reads_range(q, 3))
    CERASTES_ENSURES(r, cerastes_reads_range(q, 3) && r == q[1])
{
  return q[1];
}

int use(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
    CERASTES_ENSURES(r, r == 5)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  setrow(m[i]);
  int mid = middle(m[i]);
  return mid + m[i][2];
}
