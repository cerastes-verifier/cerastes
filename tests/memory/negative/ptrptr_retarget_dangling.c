// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/ptrptr_retarget_block.c
// Pins: 0040 item 2 (0032 item 6): the dereference of the pointer loaded from *pp after y's block reads through a dangling pointer: permission (not lifetime: a dereference's permission check is the liveness check)
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
  x += **pp;
  return x;
}
