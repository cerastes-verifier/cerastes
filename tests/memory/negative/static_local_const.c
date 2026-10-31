// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/memory/static_local_unused.c
// Pins: 0041 items 2 and 12 (question 78): a static const local is a const object (its uses fold): rejected, naming M4.6
#include <cerastes.h>

int get(void)
{
  static const int k = 9;
  return k;
}
