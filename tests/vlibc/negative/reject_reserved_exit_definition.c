// expect: rejected cerastes-annotation_reserved_library @ L9
// mode: c23, c++20
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 6 (question 117; C23 7.1.3): a user definition of a reserved name is rejected after the header
// The program may not define the library's exit.
#include <stdlib.h>
#include <cerastes.h>

void exit(int status)
{
  (void) status;
  abort();
}

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
