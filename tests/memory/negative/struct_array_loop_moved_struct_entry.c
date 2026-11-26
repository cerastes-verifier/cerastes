// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_moved_struct.c
// Pins: 0064 item 15 (Part D, step B): the row of the PART at the loop's entry for a written owns(*c): the function holds only reads(*p), so the structure's array part is not owned where the invariant needs it: one permission error at the invariant (the existing chain_loop twin fails on the user's value conjunct only; its parameter is not moved)
// Pre-registered from the auditor's probe audit-D2/w04_fraction.c function g (permission at the invariant line, "enters this loop without owning `c->a`"), before any run of this file as a test; the loop reads c->a[1] so that c is used and EDG's #550 stays quiet in C++20
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int peek(const struct S *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(cerastes_reads(*p))
{
  const struct S *c = p;
  int v = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_owns(*c))  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    v = c->a[1];
    c = p;
    i = i + 1;
  }
  return v - v;
}
