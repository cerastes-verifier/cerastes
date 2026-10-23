// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 5 (owner, 2026-10-23): == and != between an enumeration and an operand the compilers do not warn about: a constant >= 0, bool, long, unsigned char, a negative-valued enumeration, two enumerations, an explicit conversion
#include <cerastes.h>

#include <stdbool.h>

enum color { RED, GREEN, BLUE };
enum neg { NA = -1, NB };

int eq_const(enum color c)
    CERASTES_ENSURES(r, r == ((int)c == 1))
{
  return c == 1;
}


int eq_bool(enum color c, bool b)
    CERASTES_ENSURES(r, r == ((int)c == (int)b))
{
  return c == b;
}


int eq_long(enum color c, long l)
    CERASTES_ENSURES(r, r == ((long)c == l))
{
  return c == l;
}


int eq_uchar(enum color c, unsigned char uc)
    CERASTES_ENSURES(r, r == ((int)c == (int)uc))
{
  return c == uc;
}


int eq_negative_enum(enum neg n, int x)
    CERASTES_ENSURES(r, r == ((int)n == x))
{
  return n == x;
}


int eq_enums(enum color c, enum color d)
    CERASTES_ENSURES(r, r == ((int)c == (int)d))
{
  return c == d;
}


int eq_cast_int(enum color c, int x)
    CERASTES_ENSURES(r, r == ((int)c == x))
{
  return (int)c == x;
}


int eq_enumerator(int x)
    CERASTES_ENSURES(r, r == (1 == x))
{
  return GREEN == x;
}


int eq_cast_back(enum color c, int x)
    CERASTES_REQUIRES(0 <= x && x <= 3)
    CERASTES_ENSURES(r, r == ((int)c == x))
{
  return c == (enum color)x;
}
