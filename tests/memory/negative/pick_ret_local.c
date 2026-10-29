// expect: error lifetime @ L17
// mode: c23, c++20
// twin: tests/memory/pick_owns.c
// Pins: 0037 item 3: a returned pointer is a use at its return (passes there), the caller's first use of a result whose object has ended fails lifetime
#include <cerastes.h>
#include <stddef.h>

int *dangle(void)
{
  int x = 1;
  int *p = &x;
  return p;
}

int use_dangle(void)
{
  int *t = dangle();
  return t != NULL;
}
