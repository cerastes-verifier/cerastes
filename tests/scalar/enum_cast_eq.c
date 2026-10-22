// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 item 6(i) and 9: x == (enum color)2 compares an int with a cast enumeration value, which is the folded constant 2 in both modes
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int is_two(int x)
    CERASTES_ENSURES(r, (x == 2 && r == 1) || (x != 2 && r == 0))
{
  if (x == (enum color)2) {
    return 1;
  }
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a = is_two(2);
  int b = is_two(a);
  int c = is_two(a + 1);
  return a + b + c;
}
