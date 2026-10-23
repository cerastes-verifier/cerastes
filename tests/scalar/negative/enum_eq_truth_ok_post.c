// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_eq_truth_ok.c
// Pins: 0022 item 5: c == !x is not (int)c != !x
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
enum neg { NA = -1, NB };

int eq_not(enum color c, int x)
    CERASTES_ENSURES(r, r == ((int)c != !x))
{
  return c == !x;
}
