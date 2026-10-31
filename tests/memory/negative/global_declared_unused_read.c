// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/global_declared_unused.c
// Pins: 0041 items 2 and 6: the same global read by a function whose contract does not name it: permission
#include <cerastes.h>

int counter = 0;

int get(void)
{
  return counter;
}
