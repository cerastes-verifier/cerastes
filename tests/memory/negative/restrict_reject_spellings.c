// expect: rejected cerastes-unsupported_annotation @ L19
// expect: rejected cerastes-unsupported_annotation @ L26
// expect: rejected cerastes-unsupported_annotation @ L33
// expect: rejected cerastes-unsupported_annotation @ L40
// expect: rejected cerastes-unsupported_annotation @ L47
// expect: rejected cerastes-unsupported_annotation @ L54
// expect: rejected cerastes-unsupported_annotation @ L63
// expect: rejected cerastes-unsupported_annotation @ L70
// expect: frontend-warning declared_but_not_referenced @ L70
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19: every spelling and placement of a restrict parameter on a definition is rejected, once per function, at the definition's own head, called or not: restrict, __restrict, __restrict__, int p[restrict] (with owns_range(p, 1)), a typedef'd restricted pointer (the row is the definition's head, not the typedef), a second parameter alone restrict, a plain prototype followed by a restrict definition (the row is the definition's), and a static definition that nothing calls
// Pre-registered from record 0064 item 19 as settled for question 338 (edg-backend's report 3, cases C1 to C7, B5b, D1), before the run: eight rows, each at a definition's head
#include <cerastes.h>

typedef int *restrict rp;

int by_restrict(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return *p;
}

int by_underscores(int *__restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return *p;
}

int by_underscores_both(int *__restrict__ p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return *p;
}

int by_array(int p[restrict])  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, 1))
    CERASTES_ENSURES(r, cerastes_owns_range(p, 1))
{
  return p[0];
}

int by_typedef(rp p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return *p;
}

int second_only(int *q, int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*q) && cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*q) && cerastes_owns(*p))
{
  return *q;
}

int later(int *p);

int later(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return *p;
}

static int not_called(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return *p;
}
