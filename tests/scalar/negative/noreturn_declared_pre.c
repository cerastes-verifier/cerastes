// expect: error precondition @ L16
// mode: c23, c++20
// twin: tests/scalar/noreturn_declared.c
// Pins: 0015 item 4: the requires of a function that does not return is checked at the call
// die(0) violates code != 0; the call still ends the path afterwards.
#include <cerastes.h>

[[noreturn]] void die(int code)
    CERASTES_REQUIRES(code != 0);

int lookup(int key)
    CERASTES_REQUIRES(0 <= key && key <= 100)
    CERASTES_ENSURES(r, r == key * 2 && key <= 50)
{
  if (key > 50) {
    die(0);
  }
  return key * 2;
}
