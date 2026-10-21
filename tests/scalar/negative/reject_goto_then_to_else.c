// expect: rejected cerastes-unsupported_statement @ L13
// mode: c23, c++20
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 2: a goto from the then-branch into the else-branch (a different block, later)
// Neither branch encloses the other; the jump would skip the condition's else-facts.
#include <cerastes.h>

int branches(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  int y = 0;
  if (x > 5) {
    goto inside;
  } else {
    y = 1;
inside:
    y += 1;
  }
  return y;
}
