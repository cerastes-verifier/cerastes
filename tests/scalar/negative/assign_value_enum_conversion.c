// expect: error conversion @ L14
// mode: c23, c++20
// twin: tests/scalar/assign_value_enum.c
// Pins: 0018 item 5: n may be 4, which is not a value of enum color (0 to 3)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int ordinal(int n)
    CERASTES_REQUIRES(0 <= n && n <= 4)
    CERASTES_ENSURES(r, r == 2 * n)
{
  enum color c = RED;
  int v = (int)(c = (enum color)n);
  return v + (int)c;
}
