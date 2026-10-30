// expect: rejected cerastes-unsupported_expression @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_const_middle.c
// Pins: 0040 item 2 (audit B2): the const-laundering pin: (const int **)pp; *cpp = &c; *p = 2 writes the const object c with no compiler diagnostic; the cast is rejected, so check(c == 2) can never verify
#include <cerastes.h>
#include <stddef.h>

int launder(void)
{
  const int c = 1;
  int *p = 0;
  int **pp = &p;
  const int **cpp = (const int **)pp;
  *cpp = &c;
  *p = 2;
  CERASTES_CHECK(c == 2);
  return 0;
}
