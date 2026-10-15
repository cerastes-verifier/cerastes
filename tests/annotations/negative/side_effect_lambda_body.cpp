// expect: frontend-error cerastes-annotation_side_effect @ L15
// expect: native-ok
// twin: tests/annotations/statements/check_lambda_capture.cpp
// Mutation of the twin: a side effect in the body of a lambda written in the argument.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int fill(int* a, int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  while (i < n) {
    a[i] = 0;
    i += 1;
  }
  CERASTES_CHECK(cerastes::forall([&](int k) { n += 1; return !(0 <= k && k < n) || a[k] == 0; }));
  CERASTES_CHECK(cerastes::forall([=](int k) { return !(0 <= k && k < n) || a[k] == 0; }));
  CERASTES_CHECK(cerastes::forall([](int k, const int* p, int m) { return !(0 <= k && k < m) || p[k] == 0; }, a, n));
  return i;
}
