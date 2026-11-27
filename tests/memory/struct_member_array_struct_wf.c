// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 11 and 12 (Part C2, audit round C1's finding 5 for an array member of structures; f__wf's ghosts, one per leaf path and slot): the control of negative/struct_member_array_struct_wf_division.c. f__wf ties no pre-state ghost of an element of an array member to its post-state one (the listed false alarm of negative/struct_member_array_struct_wf_tie_false_alarm.c), so each clause states its own guard: the requires has p->v[0].x != 0 before 10 / p->v[0].x, and the ensures has p->v[0].x != 0 before its own division, under reads(*p) in both; the body changes nothing, so the ensures' guard holds at the return
// Pre-registered from record 0064 items 11 and 12 and the audit's finding 5 (w04's shape, the guard in each clause), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

int f(struct T *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->v[0].x != 0 && 10 / p->v[0].x > 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && p->v[0].x != 0 && (r == 0 || r == 10 / p->v[0].x))
{
  return p->v[0].x * 0;
}
