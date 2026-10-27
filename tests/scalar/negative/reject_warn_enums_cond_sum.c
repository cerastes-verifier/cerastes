// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R8: a ?: of two enumerators of one enumeration, added to an enumerator of another (the arm is an int in C)
#include <cerastes.h>

enum color { RED, GREEN };
enum nat { NA, NB };

int test(int x, int y)
{
  return RED + (x ? NA : NB);
}
