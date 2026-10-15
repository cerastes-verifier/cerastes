// expect: golden
// expect: native-ok
// Pins: owns, reads, old and owns_range called in contracts: the callee is a
// cerastes:: function template instance, arguments are field accesses through
// a pointer parameter (c->val), and old(c->val) is a call, not a special node.
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

int get(const cell* c) CERASTES_REQUIRES(cerastes::reads(c->val)) CERASTES_ENSURES(r, r == c->val)
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
