// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/global_owns_call.c
// Pins: 0041 item 6: a function whose contract names no global holds no permission to g: the compound assignment reads g without owning it
#include <cerastes.h>

static int g = 3;

void bump(void)
{
  g += 1;
}
