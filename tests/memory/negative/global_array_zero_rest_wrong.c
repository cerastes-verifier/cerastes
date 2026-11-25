// expect: error check @ L12
// mode: c23, c++20
// twin: tests/memory/global_array_values.c
// Pins: 0063 item 9: a partial list {5} leaves the zero rest: part[3] is 0, not 1
// Pre-registered from record 0063 item 9 (Part B), before any run: the one error
#include <cerastes.h>

static int part[4] = {5};

int main(void)
{
  CERASTES_CHECK(part[3] == 1);  // <- here
  return part[0] - 5;
}
