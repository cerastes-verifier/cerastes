// expect: rejected cerastes-annotation_reserved_library @ L8
// mode: c23, c++20
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 6 (question 117): the same without <stdlib.h>, where the declaration is the program's own
// A definition returns: nothing in the unit tells the encoder this is not the library's exit.
#include <cerastes.h>

void exit(int status)
{
  (void) status;
}

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
