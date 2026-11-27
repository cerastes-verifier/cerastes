// expect: error precondition @ L23
// mode: c23, c++20
// twin: tests/memory/array_rows_forall_beside.c
// Pins: 0064 item 7 (audit round E1, findings 2 and 4; the auditor's j83): the callee's quantifier requires every element of q to be zero, and the caller passes m[0], the row {1, 2, 3}: the precondition of the callee fails at the call, once. The positive passes m[1], {0, 0, 0}
// Pre-registered from record 0064 item 7 (Part E), before any run: precondition at the call
#include <cerastes.h>

CERASTES_PURE bool z(int i, int *p)
{
  return i < 0 || i >= 3 || p[i] == 0;
}

int callee(int *q)
    CERASTES_REQUIRES(cerastes_reads_range(q, 3) && cerastes_forall(z, q))
    CERASTES_ENSURES(r, cerastes_reads_range(q, 3) && r == 0)
{
  return q[2];
}

int caller(void)
{
  int m[2][3] = {{1, 2, 3}, {0, 0, 0}};
  return callee(m[0]);  // <- here
}
