// expect: verified
// expect: native-ok
// Pins: 0015 item 5: an exit in a namespace of the user is not the library's: it returns
// n::exit has a contract and no noreturn trust; the call returns and the postcondition holds as written.
#include <cerastes.h>

namespace n {
void exit(int code)
    CERASTES_ENSURES(true);
}

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 5) {
    n::exit(1);
  }
  return x;
}
