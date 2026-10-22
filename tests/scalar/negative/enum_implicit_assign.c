// expect: error conversion @ L22
// c-only: C++ refuses an implicit conversion from an integer to an enumeration; C allows it and cerastes checks the value
// twin: tests/scalar/enum_implicit.c
// Pins: 0018 item 5: x may be 4 in the assignment, which is not one of the values 0 to 3 of enum color
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color init_from(int x)
    CERASTES_REQUIRES(0 <= x && x <= 3)
    CERASTES_ENSURES(r, (int)r == x)
{
  enum color c = x;
  return c;
}

enum color assign_from(int x)
    CERASTES_REQUIRES(0 <= x && x <= 4)
    CERASTES_ENSURES(r, (int)r == x)
{
  enum color c = RED;
  c = x;
  return c;
}

enum color fixed(void)
    CERASTES_ENSURES(r, r == BLUE)
{
  enum color c = 2;
  return c;
}

int takes(enum color c)
    CERASTES_ENSURES(r, r == (int)c)
{
  return (int)c;
}

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 3)
    CERASTES_ENSURES(r, r == x)
{
  return takes(x);
}

enum color one(void)
    CERASTES_ENSURES(r, r == GREEN)
{
  return 1;
}

int total(void)
    CERASTES_ENSURES(r, r == 12)
{
  enum color a = init_from(0);
  enum color b = init_from(3);
  enum color c = assign_from(0);
  enum color d = assign_from(3);
  enum color e = fixed();
  enum color f = one();
  int g = use(0);
  int h = use(3);
  return (int)a + (int)b + (int)c + (int)d + (int)e + (int)f + g + h;
}
