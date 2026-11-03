// expect: verified
// expect: native-ok
// Pins: 0044 item 6: C++ get(T{}) with a nested member: T{} zeroes every leaf, nested ones included, in the argument's tuple
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct T {
  S s;
  int z;
};

int get(T t)
    CERASTES_ENSURES(r, r == t.s.y)
{
  return t.s.y;
}

int use()
    CERASTES_ENSURES(r, r == 0)
{
  return get(T{});
}
