// expect: error lifetime @ L11
// mode: c23, c++20
// twin: tests/memory/live_result.c
// Pins: 0037 item 3 and consequences: returning a parameter is a use; without live(p) in the requires the use fails lifetime
#include <cerastes.h>
#include <stddef.h>

int *id(int *p)
    CERASTES_ENSURES(r, r == p)
{
  return p;
}
