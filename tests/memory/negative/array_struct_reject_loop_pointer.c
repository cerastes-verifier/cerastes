// expect: rejected cerastes-unsupported_expression @ L18
// mode: c23, c++20
// twin: tests/memory/array_struct_loop.c
// Pins: 0064 item 10 (lifted by M4.5e1e2 (continuation), Part C or M4.8): a loop invariant that reads an element of a POINTER to a structure (p[0].x) stays rejected, while v[e].x of a named local array is accepted (the positive's member_inv); the lowering's check refuses it before any loop place exists. The tag is the loop-invariant pins' (range_loop_reject_index_reads_memory.c), to confirm at the first run [Part C1, Flips C: re-pointed, the row L18 is UNCHANGED on the joined build j2a (edg-backend report 7): this function holds only a structure atom (owns(*p)), no range, so p[0].x is still not a place the loop knows; the message now tells the user to write p->x, and the element read through a range of structures in a loop invariant is lifted by Part C1 (range_struct_loop.c); a structure atom's p[e] belongs to M4.5e2]
// Pre-registered from the lowering's report 3 (question 1), before any run: rejected
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int f(struct P *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(p[0].x >= 0 && 0 <= i && i <= 1)  // <- here
  CERASTES_DECREASES(1 - i)
  while (i < 1) {
    i = i + 1;
  }
  return i;
}
