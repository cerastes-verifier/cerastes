// expect: rejected cerastes-empty_body @ L11
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R13: the ; on the next line: clang is silent, GCC warns only with -Wextra, but the IL does not record the line of the `)`, so it is rejected too (listed as broader)
#include <cerastes.h>

int test(int x, int n)
    CERASTES_REQUIRES(0 <= x && x < 100 && 0 <= n && n < 100)
{
  if (x > 0)
    ;
  return x;
}
