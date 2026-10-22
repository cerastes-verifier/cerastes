// expect: error overflow @ L39
// mode: c23, c++20
// twin: tests/scalar/enum_arith.c
// Pins: 0018 item 6(iv): BIG is INT_MAX as an int in both modes, so BIG + 1 overflows
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
enum big { BIG = 2147483647 };

int next_of_green(void)
    CERASTES_ENSURES(r, r == 2)
{
  return GREEN + 1;
}

int before_red(void)
    CERASTES_ENSURES(r, r == -1)
{
  return RED - 1;
}

int add_blue(int x)
    CERASTES_REQUIRES(x <= 2147483644)
    CERASTES_ENSURES(r, r == x + 3)
{
  return x + BLUE + 1;
}

int sub_blue(int x)
    CERASTES_REQUIRES(-2147483646 <= x)
    CERASTES_ENSURES(r, r == x - 2)
{
  return x - BLUE;
}

int below_big(void)
    CERASTES_ENSURES(r, r == 2147483646)
{
  return BIG + 1;
}

int add_edge(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return add_blue(2147483644);
}

int sub_edge(void)
    CERASTES_ENSURES(r, r == -2147483647 - 1)
{
  return sub_blue(-2147483646);
}
