// expect: rejected cerastes-unsupported_statement @ L12
// expect: rejected cerastes-unsupported_statement @ L15
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: goto and its label are rejected
// Assumption A1 (INDEX.md): each rejected statement has its own diagnostic: the goto and the label.
#include <cerastes.h>

int jump(int x)
{
  if (x > 0) {
    goto done;
  }
  x = 1;
done:
  return x;
}
