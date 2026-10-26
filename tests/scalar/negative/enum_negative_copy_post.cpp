// expect: error postcondition @ L12
// twin: tests/scalar/enum_negative_copy.cpp
// Pins: 0024 item 5: enum_negative_copy.cpp with first() claiming 0: -1 is the value of A
#include <cerastes.h>

enum E { A = -1, B };

enum E first(void)
    CERASTES_ENSURES(r, (int)r == 0)
{
  enum E y = A;
  return y;
}

int second(void)
    CERASTES_ENSURES(r, r == 0)
{
  enum E z = B;
  return (int)z;
}

int sum(void)
    CERASTES_ENSURES(r, r == -1)
{
  enum E y = A;
  enum E z = B;
  return (int)y + (int)z;
}
