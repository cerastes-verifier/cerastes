// expect: error check @ L18
// twin: tests/scalar/noreturn_namespaced_exit.cpp
// Pins: 0015 item 5: code after a call of n::exit is reachable, so its check is made
// std::exit would make the path unreachable.
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
    CERASTES_CHECK(x <= 5);
  }
  return x;
}
