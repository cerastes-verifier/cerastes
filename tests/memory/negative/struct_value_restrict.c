// expect: rejected cerastes-unsupported_annotation @ L13
// c-only: compound literal objects and restrict do not exist in C++ (decision 0044 items 7 and 11)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326, the owner): every restrict parameter is rejected, unsupported_annotation, once per function at its head; drop the keyword to verify the function (tests/memory/restrict_dropped.c). Re-expected from verified by Flips R. It verified before as: 0044 item 11: a restrict int * beside a by-value struct whose members are all scalars is accepted (no int * can be formed from them)
// Pre-registered from record 0064 item 19 (Part R, Flips R), before the run: one row per function with a restrict parameter, at its head line
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(int *restrict p, struct S s)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == s.x && r == s.y)
{
  *p = s.x;
  return s.y;
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a = 0;
  struct S s = {1, 2};
  return f(&a, s);
}
