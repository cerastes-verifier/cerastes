// expect: verified
// expect: native-ok
// mode: c23
// c-only: array compound literals are C only (a GNU extension in C++; item 12 rejects them there)
// Pins: 0063 item 12 (flip A of M4.5e1e1 (the plan's section 10), was array_reject_compound_literal.c): an array compound literal subscripted, (int[]){1, 2, 3}[0]: a hidden array local of the block, written inside the expression, its decay subscripted; unchanged, line for line (twin: negative/array_compound_literal_index_high.c)
// Pre-registered from record 0063 item 12 (Part D), before any run: verifies
#include <cerastes.h>

int f(void)
{
  return (int[]){1, 2, 3}[0];
}
