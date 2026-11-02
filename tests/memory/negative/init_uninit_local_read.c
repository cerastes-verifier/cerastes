// expect: error uninitialized @ L13
// mode: c23, c++20
// twin: tests/memory/init_uninit_local.c
// Pins: 0042 item 3: the marker does not weaken 0019: a path that does not write x still reads it uninitialized
#include <cerastes.h>

int pick(int c)
{
  CERASTES_UNINIT int x;
  if (c != 0) {
    x = 1;
  }
  return x;  // <- here
}
