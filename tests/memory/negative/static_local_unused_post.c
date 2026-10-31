// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/static_local_unused.c
// Pins: 0041 item 12: the unused static local changes nothing: r == a + 1 fails
#include <cerastes.h>

int counter(int a)
    CERASTES_REQUIRES(a < 100)
    CERASTES_ENSURES(r, r == a + 1)
{
  [[maybe_unused]] static int count = 0;
  return a;
}
