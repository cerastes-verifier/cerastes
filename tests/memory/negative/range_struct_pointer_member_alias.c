// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/memory/range_struct_pointer_member.c
// Pins: 0064 item 7 (audit round C1, finding 4; the audit's z09): follow's program with the postcondition r == 5: q is &p[1], so the second store to p[1].v reaches q->v and the value read is 6, not 5: postcondition at the return, once (the range atom with a pointer-to-structure leaf is not vacuous: the pointer leaf's value is known)
// Pre-registered from record 0064 item 7 and the audit's finding 4 (z09: postcondition at its one-line return, measured in c23 on the Mac build of 02:23), before any run: postcondition at the return
#include <cerastes.h>
#include <stddef.h>

struct N {
  struct N *next;
  int v;
};

int follow(struct N *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == 5)
{
  (void)n;
  p[0].next = &p[1];
  p[1].v = 5;
  struct N *q = p[0].next;
  p[1].v = 6;
  return q->v;
}
