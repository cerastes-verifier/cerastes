// expect: golden
// expect: native-ok
// forall: a lambda without captures, the free variables passed as trailing arguments.
#include <cerastes.h>

// tag::contract[]
int first_nonneg(const int* a, [[maybe_unused]] int n)
  CERASTES_REQUIRES(n > 0 && cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n))
  CERASTES_ENSURES(r, r >= 0)
{
  return a[0];
}
// end::contract[]

// tag::check[]
int fill(int* a, int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  while (i < n) {
    a[i] = 0;
    i += 1;
  }
  CERASTES_CHECK(cerastes::forall([&](int k) { return !(0 <= k && k < n) || a[k] == 0; }));
  return i;
}
// end::check[]
