// expect: error null-dereference @ L15
// mode: c23, c++20
// twin: tests/memory/ptrptr_depth3.c
// Pins: 0040 item 2: null at the middle level: *ppp = NULL makes pp null, so ***ppp applies * to null
#include <cerastes.h>
#include <stddef.h>

int depth3(void)
{
  int x = 1;
  int *p = &x;
  int **pp = &p;
  int ***ppp = &pp;
  *ppp = NULL;
  ***ppp = 9;
  return x;
}
