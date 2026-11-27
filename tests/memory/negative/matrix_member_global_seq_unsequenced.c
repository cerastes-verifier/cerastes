// expect: error unsequenced @ L28
// expect: error unsequenced @ L35
// mode: c23, c++20
// twin: tests/memory/matrix_member_global_seq.c
// Pins: 0064 item 11 (0039 item 3): a callee that owns gu beside a read of gu.m[i][j] in the same expression, and two calls that own gu in one expression: the writes and the read may reach one cell. With the sequencing rows made assert true nothing else rejects either
// Pre-registered from record 0064 item 11 (Part E), before any run: one unsequenced per function, at the line of its expression
#include <cerastes.h>

struct U {
  int n;
  int m[2][3];
};

static struct U gu = {9, {{1, 2, 3}, {4, 5, 6}}};

int wr(void)
    CERASTES_REQUIRES(cerastes_owns(gu))
    CERASTES_ENSURES(r, cerastes_owns(gu) && r == 0)
{
  gu.m[1][1] = 7;
  return 0;
}

int beside(int i, int j)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(gu))
{
  return wr() < gu.m[i][j];  // <- here
}

int twice(void)
    CERASTES_REQUIRES(cerastes_owns(gu))
    CERASTES_ENSURES(r, cerastes_owns(gu))
{
  return wr() + wr();  // <- here
}
