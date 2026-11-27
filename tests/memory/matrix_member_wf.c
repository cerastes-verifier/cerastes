// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 11 and 14 (Part E, question 362: contract reads p->m[e][e2] of a matrix member are built in slice 3; the form of struct_member_array_struct_wf.c, the guard in each clause: the missing cross-state tie of question 345 applies; the control of negative/matrix_member_wf_division.c): the guard and the division read the cell in one state
// Pre-registered from record 0064 items 11 and 14 (Part E), before any run: verifies
#include <cerastes.h>

struct U {
  int m[2][3];
  int n;
};

int f(struct U *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->m[0][0] != 0 && 10 / p->m[0][0] > 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && p->m[0][0] != 0 && (r == 0 || r == 10 / p->m[0][0]))
{
  return p->m[0][0] * 0;
}
