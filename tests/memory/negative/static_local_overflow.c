// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/memory/static_local_counter.c
// Pins: 0041 item 12: the value of a static local is unknown on entry (only its range is known): count += 1 may overflow
#include <cerastes.h>

void count_up(void)
{
  static int count = 0;
  count += 1;
}
