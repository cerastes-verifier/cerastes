// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 1: bitwise operators in requires, ensures and check clauses (constant masks)
#include <cerastes.h>

unsigned set_flag(unsigned u)
    CERASTES_ENSURES(r, r == (u | 4u))
{
  unsigned out = u | 4u;
  CERASTES_CHECK(out == (u | 4u));
  return out;
}

// Clearing a bit with a constant mask: ~4u is 4294967291u, so the result cannot exceed it.
unsigned clear_flag(unsigned u)
    CERASTES_ENSURES(r, r == (u & ~4u) && r <= 4294967291u)
{
  unsigned out = u & ~4u;
  CERASTES_CHECK(out <= 4294967291u);
  return out;
}

int odd_to_even(int x)
    CERASTES_REQUIRES(0 <= x && (x & 1) == 1 && x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1 && (r & 1) == 0)
{
  CERASTES_CHECK((x & 1) == 1);
  return x + 1;
}

unsigned set_flag_value(void)
    CERASTES_ENSURES(r, r == 12u)
{
  return set_flag(8u);
}

unsigned set_flag_again(void)
    CERASTES_ENSURES(r, r == 4u)
{
  return set_flag(4u);
}

unsigned clear_flag_value(void)
    CERASTES_ENSURES(r, r == 3u)
{
  return clear_flag(7u);
}

unsigned clear_flag_all(void)
    CERASTES_ENSURES(r, r == 4294967291u)
{
  return clear_flag(4294967295u);
}

unsigned clear_flag_set(void)
    CERASTES_ENSURES(r, r == 0u)
{
  return clear_flag(4u);
}

int odd_to_even_value(void)
    CERASTES_ENSURES(r, r == 2)
{
  return odd_to_even(1);
}

int odd_to_even_max(void)
    CERASTES_ENSURES(r, r == 2147483646)
{
  return odd_to_even(2147483645);
}
