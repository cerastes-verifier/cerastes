// expect: frontend-error attr_parameter_after_pack @ L10
// expect: frontend-error attr_parameter_after_pack @ L10
// expect: native-ok
// twin: tests/annotations/contracts/pack_last.cpp
// Mutation of the twin: a contract naming a parameter that follows a function parameter pack (an instance's parameter numbers shift): function template; requires and ensures both name y, so two errors at the line (the call is dropped from use(): a leading pack cannot be deduced).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class... Ts>
int m(Ts... xs, int y) CERASTES_REQUIRES(y >= 0) CERASTES_ENSURES(r, r >= y)
{
  return y + sizeof...(xs);
}

int m2(int y, auto... xs) CERASTES_REQUIRES(y >= 0)
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
  return m2(1, 2) + h.m3(1, 2, 3);
}
