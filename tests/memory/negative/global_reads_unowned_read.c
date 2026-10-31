// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/global_reads_frame.c
// Pins: 0041 item 6: a read of g in a function whose contract names no global: permission (reads g without owning it)
#include <cerastes.h>

static int g = 3;

int peek(void)
{
  return g;
}
