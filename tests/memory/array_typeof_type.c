// expect: verified
// expect: native-ok
// mode: c23
// c-only: typeof is a C23 keyword
// Pins: 0063 item 14 (flip A of M4.5e1e1 (the plan's section 10), was array_reject_typeof_type.c): typeof of an array type name is the array type, its bound checked in the type operand; unchanged, line for line (twin: negative/array_typeof_type_index_high.c)
// Pre-registered from record 0063 item 14 (Part D), before any run: verifies
#include <cerastes.h>

int f(void)
{
  typeof(int[3]) b = {1, 2, 3};
  return b[0];
}
