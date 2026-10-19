// expect: golden
// expect: native-ok
// twin-exempt: header coverage test; every rule it touches (requires, ensures, invariant, check, fold, predicate, pure, spec functions) has its own negatives named by the per-attribute positives
// Pins: <cerastes.h> in C++: every CERASTES_* macro used once (each expands
// to its attribute under __CERASTES__), the header included twice (guard),
// and every cerastes:: spec function used (old in an ensures: it is an error in a predicate body, R4e).
#include <cerastes.h>
#include <cerastes.h>

CERASTES_PREDICATE bool spec_probe(const int* p, unsigned n)
{
  return cerastes::owns(*p) && cerastes::reads(*p) && cerastes::owns_range(p, n) &&
         *p == 0 && cerastes::forall([](int i) { return i >= 0; });
}

CERASTES_PURE int plus1(int x) CERASTES_REQUIRES(x > 0) CERASTES_ENSURES(r, r == cerastes::old(x) + 1)
{
  return x + 1;
}

int everything(int n) CERASTES_REQUIRES(n >= 0) CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) while (i < n) {
    i += 1;
  }
  CERASTES_CHECK(i == n);
  CERASTES_FOLD(spec_probe(&i, 1));
  CERASTES_UNFOLD(spec_probe(&i, 1));
  return i;
}
