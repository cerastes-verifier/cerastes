// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 (Part E; the sequencing rows of a GLOBAL structure's matrix member have nothing else behind them): a call that owns the unrelated global x beside a read of gu.m[i][j] is accepted (disjoint objects). The twins in negative/matrix_member_global_seq_unsequenced.c let the callee own gu
// Pre-registered from record 0064 item 11 (Part E; the form of global_array_struct_seq.c's disjoint function), before any run: verifies
#include <cerastes.h>

struct U {
  int n;
  int m[2][3];
};

static struct U gu = {9, {{1, 2, 3}, {4, 5, 6}}};
static int x = 9;

int set_x(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*q) && *q == 5 && r == 0)
{
  *q = 5;
  return 0;
}

int disjoint(int i, int j)
    CERASTES_REQUIRES(cerastes_owns(gu) && cerastes_owns(x) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(gu) && cerastes_owns(x))
{
  return set_x(&x) < gu.m[i][j];
}
