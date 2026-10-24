// expect: error postcondition @ L24
// mode: c23, c++20
// twin: tests/scalar/bit_enumerator.c
// Pins: 0023 item 4: GREEN ^ 3 is 1 ^ 3 = 2, not 3 (GREEN ^ BLUE would be 3 for | as well)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int or_green(int x)
    CERASTES_ENSURES(r, r == (x | 1))
{
  return x | GREEN;
}

int not_green(void)
    CERASTES_ENSURES(r, r == -2)
{
  return ~GREEN;
}

int xor_green_three(void)
    CERASTES_ENSURES(r, r == 3)
{
  return GREEN ^ 3;
}

int or_green_two(void)
    CERASTES_ENSURES(r, r == 3)
{
  return GREEN | 2;
}

int or_green_value(void)
    CERASTES_ENSURES(r, r == 5)
{
  return or_green(4);
}

int and_blue_value(void)
    CERASTES_ENSURES(r, r == 2)
{
  return BLUE & 3;
}

int or_red_six(void)
    CERASTES_ENSURES(r, r == 6)
{
  return RED | 6;
}
