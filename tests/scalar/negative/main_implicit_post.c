// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/scalar/main_implicit.c
// Pins: 0012 item 9: the implicit return 0 at the end of main is checked against main's ensures
#include <cerastes.h>

int main(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 1;
  if (x > 5) {
    return 3;
  }
  CERASTES_CHECK(x == 1);
}
