// expect: error termination @ L16
// mode: c23, c++20
// twin: tests/memory/measure_reads_memory.c
// Pins: 0039 item 8: without the decrement *p is the same at the recursive call: termination at the call
#include <cerastes.h>
#include <stddef.h>

int down(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p >= 0 && *p < 100)
    CERASTES_DECREASES(*p)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  if (*p == 0) {
    return 0;
  }
  return down(p);
}
