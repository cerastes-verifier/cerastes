// expect: rejected cerastes-unsupported_annotation @ L18
// expect: frontend-warning set_but_not_used @ L14
// mode: c23, c++20
// twin: tests/memory/matrix_member_wf.c
// Pins: 0064 item 14 and question 346's limit (the coordinator's ruling 362): cerastes_old of an element of a global structure's member (gu.m[1][1]) stays rejected, as cerastes_old(gt.v[1].x) does (struct_member_array_struct_reject_global_element.c)
// Pre-registered from record 0064 item 14 (Part E), before any run: rejected (tag by struct_member_array_struct_reject_global_element.c); corrected once after the first run on j4a (question 160): the front end's #550 warning on `gu` (read only by the contract) is registered, the header grew by one line so the lines moved by +1
#include <cerastes.h>

struct U {
  int m[2][3];
  int n;
};

static struct U gu = {{{1, 2, 3}, {4, 5, 6}}, 0};

void gf(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && cerastes_owns(gu))
    CERASTES_ENSURES(cerastes_owns(*q) && cerastes_owns(gu) && gu.m[1][1] == cerastes_old(gu.m[1][1]) && *q == 99)  // <- here
{
  *q = 99;
}
