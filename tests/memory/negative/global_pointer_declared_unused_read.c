// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/global_pointer_declared_unused.c
// Pins: 0041 items 2 and 6: the pointer global read by a function whose contract does not name it: permission
#include <cerastes.h>

int *gp;

int *get(void)
{
  return gp;
}
