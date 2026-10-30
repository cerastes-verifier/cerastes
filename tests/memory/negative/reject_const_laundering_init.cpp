// expect: rejected bad_initializer_type @ L13
// mode: c++20
// twin: tests/memory/ptrptr_const_middle.c
// Pins: 0040 item 2 (P62 H1): C++ accepts int ** to const int ** with a warning (clang++ and g++ reject it); the driver makes #144 an error in C++ too
#include <cerastes.h>
#include <stddef.h>

int laundering_init(void)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  const int **cpp = pp;
  return cpp == nullptr;
}
