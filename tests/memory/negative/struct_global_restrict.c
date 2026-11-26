// expect: rejected cerastes-unsupported_annotation @ L15
// c-only: compound literal objects and restrict do not exist in C++ (decision 0044 items 7 and 11)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326, the owner): every restrict parameter is rejected, unsupported_annotation, once per function at its head; drop the keyword to verify the function (tests/memory/restrict_dropped.c). Re-expected from verified by Flips R. It verified before as: 0044 item 11: a restrict int * beside owns(g) of a struct global whose members are all scalars is accepted
// Pre-registered from record 0064 item 19 (Part R, Flips R), before the run: one row per function with a restrict parameter, at its head line
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S g;

int f(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g) && *p == 1 && r == g.x)
{
  *p = 1;
  return g.x;
}

int main(void)
{
  int a = 0;
  return f(&a);
}
