// expect: golden
// expect: native-ok
// owns, reads, old and owns_range in contracts.
#include <cerastes.h>

// tag::spec[]
struct cell {
  int val;
};

void bump(cell* c)
  CERASTES_REQUIRES(cerastes::owns(c->val) && c->val < 100)
  CERASTES_ENSURES(cerastes::owns(c->val) && c->val == cerastes::old(c->val) + 1)
{
  c->val += 1;
}

int get(const cell* c) CERASTES_REQUIRES(cerastes::reads(c->val)) CERASTES_ENSURES(r, r == c->val)
{
  return c->val;
}

int first(const int* a, unsigned n)
  CERASTES_REQUIRES(n > 0 && cerastes::owns_range(a, n))
  CERASTES_ENSURES(r, r == a[0])
{
  return n > 0 ? a[0] : 0;
}
// end::spec[]
