// expect: error lifetime @ L18
// mode: c23, c++20
// twin: tests/memory/ptrptr_retarget_block.c
// Pins: 0040 item 2: comparing the pointer loaded from *pp after y's block (a dangling pointer) with &x
#include <cerastes.h>
#include <stddef.h>

int retarget_block(int a)
    CERASTES_REQUIRES(-1000 < a && a < 1000)
{
  int x = a;
  int *p = &x;
  int **pp = &p;
  {
    int y = 7;
    *pp = &y;
  }
  if (*pp == &x) {
    x += 1;
  }
  return x;
}
