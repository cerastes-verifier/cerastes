// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 3: uninit on automatic locals without an initializer, in both positions, in a for init-statement and over several declarators; it states intent and emits nothing
#include <cerastes.h>

int pick(int c)
    CERASTES_ENSURES(r, r == 1 || r == 2)
{
  CERASTES_UNINIT int x;
  if (c != 0) {
    x = 1;
  } else {
    x = 2;
  }
  return x;
}

int trailing(int c)
    CERASTES_ENSURES(r, r == 3 || r == 4)
{
  int y CERASTES_UNINIT;
  if (c != 0) {
    y = 3;
  } else {
    y = 4;
  }
  return y;
}

int in_for(void)
    CERASTES_ENSURES(r, r == 1)
{
  int s = 0;
  CERASTES_INVARIANT(0 <= s && s <= 1)
  CERASTES_DECREASES(1 - s)
  for (CERASTES_UNINIT int i; s < 1;) {
    i = s;
    s = i + 1;
  }
  return s;
}

int many(int c)
    CERASTES_ENSURES(r, r == 5)
{
  CERASTES_UNINIT int a, b;
  a = c;
  b = 5;
  return a - c + b;
}
