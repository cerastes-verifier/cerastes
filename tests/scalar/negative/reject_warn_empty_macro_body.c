// expect: rejected cerastes-empty_body @ L12
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R13: a macro that expands to nothing leaves a null statement as the body; clang warns only for a ; written on the line of the `)`, the IL does not tell, so it is rejected (listed as broader)
#include <cerastes.h>

#define LOG(m)

int test(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
{
  if (x > 0) LOG("positive");
  return x;
}
