// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1 and 0018 item 5: an assignment of an enumeration type has the enumeration's value; the cast is checked
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int ordinal(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == 2 * n)
{
  enum color c = RED;
  int v = (int)(c = (enum color)n);
  return v + (int)c;
}
