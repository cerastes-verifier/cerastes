// expect: golden
// expect: native-ok
// Pins: contracts in variadic templates: sizeof...(Ts) of a TYPE pack is
// accepted (macro form, and the spelled-out form under __CERASTES__); the
// body may use the function parameter pack. Twin of the pack_* negatives,
// where the contract names the function parameter pack or expands a pack
// (EDG would substitute those as empty packs in every instance).
#include <cerastes.h>

template <class... Ts>
int count(Ts... args) CERASTES_REQUIRES(sizeof...(Ts) > 0) CERASTES_ENSURES(r, r > 0)
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
