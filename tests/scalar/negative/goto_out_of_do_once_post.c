// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/scalar/goto_out_of_do_once.c
// Pins: 0015 item 2: the goto path out of the do-once block reaches the return with result == -1
// The ensures claims the block always runs to its end.
#include <cerastes.h>

int guarded(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x * 2)
{
  int result = -1;
  do {
    if (x > 50) {
      goto out;
    }
    result = x * 2;
  } while (0);
out:
  return result;
}
