// expect: frontend-error attr_parameter_after_pack @ L14
// expect: native-ok
// twin: tests/annotations/contracts/pack_last.cpp
// Mutation of the twin: the same for an abbreviated function template.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class... Ts>
int m(int y, Ts... xs) CERASTES_REQUIRES(y >= 0) CERASTES_ENSURES(r, r >= y)
{
  return y + sizeof...(xs);
}

int m2(auto... xs, int y) CERASTES_REQUIRES(y >= 0)
{
  return y + sizeof...(xs);
}

template <class... Ts>
struct holder {
  int m3(int y, Ts... xs) CERASTES_REQUIRES(y >= 0) { return y + sizeof...(xs); }
};

int use()
{
  holder<int, long> h;
  return m(1, 2, 3.0) + h.m3(1, 2, 3);
}
