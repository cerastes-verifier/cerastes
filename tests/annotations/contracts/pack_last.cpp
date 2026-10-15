// expect: golden
// expect: native-ok
// Pins: contracts in variadic functions whose pack is the LAST parameter, so a
// contract may name the parameters before it (decision 0007 item 5, parameters
// after a pack): a function template, an abbreviated function template and a
// member of a class template with a pack. Twin of the pack_param_after_pack_*
// negatives, which put the pack first.
#include <cerastes.h>

template <class... Ts>
int m(int y, Ts... xs) CERASTES_REQUIRES(y >= 0) CERASTES_ENSURES(r, r >= y)
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
  return m(1, 2, 3.0) + m2(1, 2) + h.m3(1, 2, 3);
}
