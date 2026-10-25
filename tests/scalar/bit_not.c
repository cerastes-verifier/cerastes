// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 1: ~ is -a - 1 for signed and MAX - a for unsigned operands, with no obligation
#include <cerastes.h>

int not_int(int a)
    CERASTES_ENSURES(r, r == -1 - a)
{
  return ~a;
}

unsigned not_uns(unsigned a)
    CERASTES_ENSURES(r, r == 4294967295u - a)
{
  return ~a;
}

int not_not(int a)
    CERASTES_ENSURES(r, r == a)
{
  return ~~a;
}

int not_zero(void)
    CERASTES_ENSURES(r, r == -1)
{
  return not_int(0);
}

int not_minus_one(void)
    CERASTES_ENSURES(r, r == 0)
{
  return not_int(-1);
}

int not_big(void)
    CERASTES_ENSURES(r, r == -2147483647)
{
  return not_int(2147483646);
}

unsigned not_uns_zero(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return not_uns(0u);
}

unsigned not_uns_all(void)
    CERASTES_ENSURES(r, r == 0u)
{
  return not_uns(4294967295u);
}

unsigned not_uns_byte(void)
    CERASTES_ENSURES(r, r == 4294967040u)
{
  return not_uns(255u);
}
