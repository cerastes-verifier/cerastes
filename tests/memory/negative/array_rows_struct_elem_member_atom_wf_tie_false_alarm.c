// expect: error division-by-zero @ L20
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_member_atom.c
// Pins: 0064 item 7 (listed false alarm, question 345 and question 376's last sentence; audit E3a finding 8 (h); the program is correct): f__wf has no tie between the pre- and post-state ghosts of a cell of an element of an ARRAY MEMBER reached through a pointer (p->v[0].a[1] here) under reads(*p): the guard p->v[0].a[1] != 0 of the requires does not make the division of the ensures safe, so `division-by-zero` is reported at the ensures; the same contract with the guard in the clause that divides (guarded, in the twin) verifies. M4.8
// Pre-registered (test-mutator-10, Task 1/2, before any run): division-by-zero at the ensures (a false alarm: the program is correct); control: guarded in tests/memory/array_rows_struct_elem_member_atom.c
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

int unguarded(struct T *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->v[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && (r == 0 || r == 10 / p->v[0].a[1]))  // <- here
{
  return p->v[0].a[1] * 0;
}
