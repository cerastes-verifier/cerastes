// expect: error division-by-zero @ L23
// expect: error division-by-zero @ L30
// expect: error division-by-zero @ L37
// expect: error division-by-zero @ L44
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_wf.c
// Pins: 0064 items 11 and 12 (Part C2, audit round C1's finding 5 for an array member of structures; f__wf's ghosts, one per leaf path and slot, shared by no two): each function proves p->v[0].x != 0 in its requires and in its ensures and divides in that ensures, in the SAME state as the guard, by another value, so the division is by zero, once per function. other_leaf: p->v[0].y (another leaf of the same element); other_slot: p->v[1].x (the same leaf of another element); other_member: p->n (a member of the structure); other_index: p->v[j].x against the guard on p->v[i].x (the guard of another index). The old fifth row (owned: a guard in the requires and the division in an ensures under owns(*p), a body that writes 0) is not registered here: its cross-state shape cannot be told from the false alarm of negative/struct_member_array_struct_wf_tie_false_alarm.c (question to the orchestrator)
// Pre-registered from record 0064 items 11 and 12 and the audit's finding 5 (w01, w02, w03, w11's shapes over the member array, the guard and the division in one clause's state), before any run: division-by-zero at the ensures line of each function
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

int other_leaf(struct T *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->v[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && p->v[0].x != 0 && (r == 0 || r == 10 / p->v[0].y))
{
  return p->v[0].x * 0;
}

int other_slot(struct T *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->v[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && p->v[0].x != 0 && (r == 0 || r == 10 / p->v[1].x))
{
  return p->v[0].x * 0;
}

int other_member(struct T *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->v[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && p->v[0].x != 0 && (r == 0 || r == 10 / p->n))
{
  return p->v[0].x * 0;
}

int other_index(struct T *p, int i, int j)
    CERASTES_REQUIRES(cerastes_reads(*p) && 0 <= i && i < 2 && 0 <= j && j < 2 && p->v[i].x != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && 0 <= i && i < 2 && 0 <= j && j < 2 && p->v[i].x != 0 && (r == 0 || r == 10 / p->v[j].x))
{
  (void)j;
  return p->v[i].x * 0;
}
