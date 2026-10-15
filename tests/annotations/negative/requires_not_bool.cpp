// expect: frontend-error expr_not_bool @ L19
// expect: native-ok
// twin: tests/annotations/spec_functions/spec_calls.cpp
// Mutation of the twin: a contract whose type is a class that is not convertible to bool.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

struct cell {
  int val;
};

void set(cell* c, int v)
  CERASTES_REQUIRES(cerastes::owns(c->val))
  CERASTES_ENSURES(cerastes::owns(c->val) && c->val == v)
{
  c->val = v;
}

int get(const cell* c) CERASTES_REQUIRES(*c) CERASTES_ENSURES(r, r == c->val)
{
  return c->val;
}

void bump(cell* c)
  CERASTES_REQUIRES(cerastes::owns(c->val) && c->val < 100)
  CERASTES_ENSURES(cerastes::owns(c->val) && c->val == cerastes::old(c->val) + 1)
{
  c->val += 1;
}

int first(const int* a, unsigned n)
  CERASTES_REQUIRES(n > 0 && cerastes::owns_range(a, n))
  CERASTES_ENSURES(r, r == a[0])
{
  return n > 0 ? a[0] : 0;
}
