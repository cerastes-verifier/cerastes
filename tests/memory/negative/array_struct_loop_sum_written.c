// expect: error invariant-preserved @ L21
// mode: c23, c++20
// twin: tests/memory/array_struct_loop.c
// Pins: 0064 item 10: the loop writes v[i].x (7) before reading it, so a write reaches the int leaves, their values are not kept and s <= 5 * i is not preserved (the user's conjunct fails, nothing else); it is also false of the program (the first iteration adds 7 to a bound of 5)
// Pre-registered from record 0064 item 10 (Part B), before any run (revised before any run to the item's reach rule): the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 15)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(0 <= s)
  CERASTES_INVARIANT(s <= 5 * i)  // <- here
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    v[i].x = 7;
    s = s + v[i].x;
    i = i + 1;
  }
  return s;
}
