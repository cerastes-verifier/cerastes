// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/memory/static_local_counter.c
// Pins: 0041 item 12 (audit S7): the declaration emits no store (the initializer ran once, before main), so a counter that is incremented does not always return 1
#include <cerastes.h>

int count_calls(void)
    CERASTES_ENSURES(r, r == 1)
{
  static int c = 0;
  if (c < 100) {
    c++;
  }
  return c;
}
