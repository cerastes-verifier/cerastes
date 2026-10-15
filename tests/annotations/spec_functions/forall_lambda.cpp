// expect: golden
// expect: native-ok
// Pins: the owner's forall form (decision 0006 item 9): a capture-less lambda
// whose first parameter is the bound variable, free variables passed as
// trailing arguments; in requires, in ensures with a result binder, and with
// no free variables; the lambda parameters shadow the function's.
#include <cerastes.h>

int first_nonneg(const int* a, [[maybe_unused]] int n)
  CERASTES_REQUIRES(n > 0 && cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n))
  CERASTES_ENSURES(r, r >= 0)
{
  return a[0];
}

int zero_all(int* a, int n)
  CERASTES_ENSURES(r, r == n && cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] == 0; }, a, n))
{
  int i = 0;
  while (i < n) {
    a[i] = 0;
    i += 1;
  }
  return n;
}

bool small(int x) CERASTES_ENSURES(r, cerastes::forall([](int i) { return i >= 0; }) && r == (x < 10))
{
  return x < 10;
}
