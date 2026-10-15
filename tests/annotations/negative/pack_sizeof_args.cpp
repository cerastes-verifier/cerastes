// expect: frontend-error attr_function_parameter_pack @ L9
// expect: native-ok
// twin: tests/annotations/contracts/variadic_template.cpp
// Mutation of the twin: sizeof...(args) of a function parameter pack (sizeof...(Ts) of the type pack stays accepted: the twin).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class... Ts>
int count(Ts... args) CERASTES_REQUIRES(sizeof...(args) > 0) CERASTES_ENSURES(r, r > 0)
{
  return sizeof...(args);
}

#ifdef __CERASTES__
template <class... Ts>
int count2(Ts... args) [[cerastes::requires(sizeof...(Ts) > 0)]]
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
