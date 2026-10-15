// expect: frontend-error wrong_entity_for_attribute @ L18
// expect: native-ok
// twin: tests/annotations/statements/check_lambda_capture.cpp
// Mutation of the twin: a contract on a lambda (0006 item 2: not allowed).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
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
  [[maybe_unused]] auto id = [](int k) CERASTES_REQUIRES(k > 0) { return k; };
  return i;
}
