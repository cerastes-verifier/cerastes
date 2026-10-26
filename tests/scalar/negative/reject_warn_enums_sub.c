// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R8: `RED - NA`, two different enumerations (clang++ and g++ warn by default in C++20; C compilers are silent; rejected in both modes)
#include <cerastes.h>

enum color { RED, GREEN };
enum nat { NA, NB };

int test(int x)
{
  return RED - NA;
}
