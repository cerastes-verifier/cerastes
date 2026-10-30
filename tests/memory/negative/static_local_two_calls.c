// expect: error unsequenced @ L21
// mode: c23, c++20
// twin: tests/memory/static_local_counter.c
// Pins: 0041 item 12 (0028 item 13): next() + next(): both calls own the static local's cell and C does not order them: unsequenced at the later call
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

int both(void)
    CERASTES_ENSURES(r, 2 <= r && r <= 200)
{
  return next() + next();
}
