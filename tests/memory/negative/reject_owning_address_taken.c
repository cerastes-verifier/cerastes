// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 items 3 and 8: an owning local whose address is taken (a cell, not a plain variable): M4.5d2b2
// Pre-registered from record 0052 item 8: rejected unsupported_annotation at the declaration of p (tag and line guessed: the encoder may report the &p instead)
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));  // <- here
  int **pp = &p;
  free(*pp);
  return 0;
}
