// expect: golden
// expect: native-ok
// Pins: R4b: a predicate member function called only in a contract (the base of the pointer-to-member negatives)
#include <cerastes.h>

struct S {
  CERASTES_PREDICATE bool ok(int x) const { return x >= 0; }
};

int ident(const S& s, int x) CERASTES_REQUIRES(s.ok(x)) CERASTES_ENSURES(r, r == x)
{
  (void)s;
  return x;
}
