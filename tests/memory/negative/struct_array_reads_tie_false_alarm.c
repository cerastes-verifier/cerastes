// expect: error overflow @ L14
// mode: c23, c++20
// twin: tests/memory/struct_array_owns_array.c
// Pins: 0063 item 7 and audit round 3 N1 (a pinned false alarm, fail closed): under reads(p->a) a member element read in the requires and the same element read in the ensures are two states, and the encoder does not tie them (it only weakens): the sum in the ensures reports overflow though the requires bounds the element; the sibling without arithmetic (first in struct_array_owns_array.c) verifies
// Pre-registered from audit round 3 N1 and record 0063 item 7 (Part C), before any run: the one error: overflow at the line of twice (contract and body share it)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int twice(struct S *p) CERASTES_REQUIRES(cerastes_reads(p->a) && p->a[0] < 1000 && p->a[0] > -1000) CERASTES_ENSURES(r, cerastes_reads(p->a) && r == p->a[0] + p->a[0]) { return p->a[0] + p->a[0]; }  // <- here
