// expect: rejected cerastes-unsupported_expression @ L13
// mode: c++20
// twin: tests/memory/ptrptr_const_middle.c
// Pins: 0040 item 2 (audit B2): C++'s multi-level qualification conversion int ** to const int *const * is legal C++ but rejected in both modes (an M8 entry: [conv.qual] by types)
#include <cerastes.h>
#include <stddef.h>

int read_both()
{
  int x = 3;
  int *p = &x;
  int **pp = &p;
  const int *const *cq = pp;
  return **cq;
}
