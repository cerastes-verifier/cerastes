// expect: error check @ L12
// mode: c23, c++20
// twin: tests/memory/global_array_declared.c
// Pins: 0063 item 9 (0041 item 3): an array global with external linkage and no initializer (a C tentative definition may merge with another unit's, a C++ external one may be written by another unit's initializer) has unknown elements in main: values[0] == 0 is not a fact
// Pre-registered from record 0063 item 9 (Part B), before any run: the one error
#include <cerastes.h>

int values[3];

int main(void)
{
  CERASTES_CHECK(values[0] == 0);  // <- here
  return 0;
}
