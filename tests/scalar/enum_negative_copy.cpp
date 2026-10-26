// expect: verified
// expect: native-ok
// Pins: 0024 item 5: the literal rule is by value and type: a negative constant typed by an enumeration is read as before (the value -1, exact)
#include <cerastes.h>

enum E { A = -1, B };

enum E first(void)
    CERASTES_ENSURES(r, (int)r == -1)
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
