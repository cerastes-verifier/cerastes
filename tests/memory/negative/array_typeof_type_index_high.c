// expect: error out-of-bounds @ L13
// mode: c23
// c-only: typeof is a C23 keyword
// twin: tests/memory/array_typeof_type.c
// Pins: 0063 item 14: typeof(int[3]) b has the bound 3 its type operand wrote: i == 3 reaches the read
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error
#include <cerastes.h>

int f(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
{
  typeof(int[3]) b = {1, 2, 3};
  return b[i];  // <- here
}
