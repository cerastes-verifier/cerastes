// expect: golden
// expect: native-ok
// Pins: in a block-scope statement attribute a lambda may capture
// ([&] and [=], decision 0006 item 9), and the owner's free-variable form works
// in the same place.
#include <cerastes.h>

int fill(int* a, int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  while (i < n) {
    a[i] = 0;
    i += 1;
  }
  CERASTES_CHECK(cerastes::forall([&](int k) { return !(0 <= k && k < n) || a[k] == 0; }));
  CERASTES_CHECK(cerastes::forall([=](int k) { return !(0 <= k && k < n) || a[k] == 0; }));
  CERASTES_CHECK(cerastes::forall([](int k, const int* p, int m) { return !(0 <= k && k < m) || p[k] == 0; }, a, n));
  return i;
}
