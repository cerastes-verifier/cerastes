// expect: rejected subscript_out_of_range @ L12
// mode: c23, c++20
// twin: tests/memory/global_array_declared.c
// Pins: 0063 item 2 (0059 item 4, P82 (g)): a constant index outside an array global is EDG's #175, an error in verify mode (clang warns on exactly this set); a front-end error, so it stands alone in its file
// Pre-registered from record 0063 item 2 (Part B), before any run: rejected
#include <cerastes.h>

static int g[3] = {1, 2, 3};

int f(void)
{
  return g[3];  // <- here
}
