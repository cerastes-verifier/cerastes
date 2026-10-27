// expect: rejected cerastes-constant_out_of_range_compare @ L11
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R10: an enumeration compared with a value outside its range 0..3 (clang C++ only; rejected in both modes)
#include <cerastes.h>

enum e10 { A10, B10, C10, D10 };

int test(enum e10 e)
{
  return e == 7;
}
