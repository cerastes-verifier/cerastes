// expect: error check @ L12
// mode: c23, c++20
// twin: tests/memory/global_array_values.c
// Pins: 0063 item 1: sizeof(full) / sizeof(full[0]) is the element count 3, not 4
// Pre-registered from record 0063 item 1 (Part B), before any run: the one error
#include <cerastes.h>

static int full[3] = {1, 2, 3};

int main(void)
{
  CERASTES_CHECK(sizeof(full) / sizeof(full[0]) == 4);  // <- here
  return full[0] - 1;
}
