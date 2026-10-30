// expect: error null-dereference @ L14
// mode: c23, c++20
// twin: tests/memory/ptrptr_store.c
// Pins: 0040 item 2: *pp = NULL stores null into the cell p, so **pp applies * to null
#include <cerastes.h>
#include <stddef.h>

int store_null(void)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  *pp = NULL;
  **pp = 5;
  return x;
}
