// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/measure_reads_memory.c
// Pins: 0039 item 8: the measure reads *p and the contract holds only live(p): the measure is not framed (permission at the decreases)
#include <cerastes.h>
#include <stddef.h>

int unframed(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_DECREASES(*p)
{
  (void)p;
  return 0;
}
