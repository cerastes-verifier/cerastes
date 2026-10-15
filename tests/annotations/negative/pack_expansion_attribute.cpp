// expect: frontend-error attr_pack_expansion @ L16
// expect: native-ok
// twin: tests/annotations/contracts/variadic_template.cpp
// Mutation of the twin: a pack expansion of the attribute itself (spelled out: a macro cannot put the ellipsis there).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class... Ts>
int count(Ts... args) CERASTES_REQUIRES(sizeof...(Ts) > 0) CERASTES_ENSURES(r, r > 0)
{
  return sizeof...(args);
}

#ifdef __CERASTES__
template <class... Ts>
int count2(Ts... args) [[cerastes::requires(sizeof(Ts) > 0)...]]
{
  return sizeof...(args);
}
#else
template <class... Ts>
int count2(Ts... args)
{
  return sizeof...(args);
}
#endif

int use()
{
  return count(1, 2) + count2(1, 2);
}
