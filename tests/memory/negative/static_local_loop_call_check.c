// expect: error check @ L32
// mode: c23, c++20
// twin: tests/memory/static_local_loop_call_fn.c
// Pins: 0041 item 12 (audit round 2, SF1): the static local's value is unknown on entry and changed by each call: the result after three calls is not 3
#include <cerastes.h>

int f(void)
{
  static int c = 0;
  if (c < 100) {
    c++;
  }
  return c;
}

int h(void)
{
  int r = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    r = f();
    i++;
  }
  return r;
}

int main(void)
{
  int a = h();
  CERASTES_CHECK(a == 3);
  return 0;
}
