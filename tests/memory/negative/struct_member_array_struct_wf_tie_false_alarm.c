// expect: error division-by-zero @ L20
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_wf.c
// Pins: 0063 item 7 and 0064 item 7 (listed false alarm, question 345; the program is correct): f__wf has no tie between the pre- and post-state ghosts of an element of an ARRAY MEMBER (p->v[0].x here) under reads(*p) or reads(p->v): the guard p->v[0].x != 0 is stated in the requires, the division 10 / p->v[0].x in the ensures, and the two reads are different ghosts, so the well-formedness check finds the divisor possibly zero: division-by-zero at the ensures, once (fail closed). The same program over a scalar member (p->n), over a leaf of a structure member (p->a.x), over a pointer to the structure, and over a global array (reads(g)) verifies. A guard stated in the same clause as the division verifies (the twin). When the tie is made this file is retired and the twin's rewrite is joined by a positive of this program
// Pre-registered from record 0063 item 7 ("f__wf does not tie a member element's pre- and post-state ghosts") and 0064 item 7, before any run: division-by-zero at the ensures
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
    CERASTES_REQUIRES(cerastes_reads(*p) && p->v[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && (r == 0 || r == 10 / p->v[0].x))
{
  return p->v[0].x * 0;
}
