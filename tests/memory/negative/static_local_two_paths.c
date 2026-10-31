// expect: error unsequenced @ L33
// mode: c23, c++20
// twin: tests/memory/static_local_counter.c
// Pins: 0041 item 12: first() + second() where both reach next: each holds the static local across its body (an implicit atom), so the two calls are unordered memory calls on one cell
#include <cerastes.h>

int next(void)
    CERASTES_ENSURES(r, 1 <= r && r <= 100)
{
  static int count = 0;
  if (count < 0 || count > 99) {
    count = 0;
  }
  count += 1;
  return count;
}

int first(void)
    CERASTES_ENSURES(r, 1 <= r && r <= 100)
{
  return next();
}

int second(void)
    CERASTES_ENSURES(r, 1 <= r && r <= 100)
{
  return next();
}

int both(void)
    CERASTES_ENSURES(r, 2 <= r && r <= 200)
{
  return first() + second();
}
