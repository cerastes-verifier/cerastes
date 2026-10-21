// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 4: a function declared [[noreturn]] and defined in another unit is used through its declaration
// die() has a requires and no body here; after the call the path is unreachable (decision 0011 item 6: the other
// unit is trusted).
#include <cerastes.h>

[[noreturn]] void die(int code)
    CERASTES_REQUIRES(code != 0);

int lookup(int key)
    CERASTES_REQUIRES(0 <= key && key <= 100)
    CERASTES_ENSURES(r, r == key * 2 && key <= 50)
{
  if (key > 50) {
    die(1);
  }
  return key * 2;
}
