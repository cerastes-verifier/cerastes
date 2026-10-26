// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0025 item 1 (finding A): the neighbours of the uninitialized_guarded_* tests that verify: x holds a value when the guarded operand reads it
#include <cerastes.h>

int g(int v)
    CERASTES_ENSURES(r, r == v);

int f(int a)
    CERASTES_ENSURES(r, r == a);

// x is initialized, so reading it in the guarded operand is fine
int initialized(int a)
    CERASTES_ENSURES(r, r == 4)
{
  int x = 0;
  int y = (x = (g(f(a) > 0 && x > 0), 2));
  return y + x;
}

// x is written before the guarded read
int written_first(int a)
    CERASTES_ENSURES(r, r == 4)
{
  int x;
  x = 1;
  int y = (x = (g(f(a) > 0 && x > 0), 2));
  return y + x;
}

// the read follows the write in the same expression
int written_in_comma(int a)
    CERASTES_ENSURES(r, r == 3)
{
  int x;
  int y = (x = 1, g(f(a) > 0 && x > 0), x + 2);
  return y;
}
