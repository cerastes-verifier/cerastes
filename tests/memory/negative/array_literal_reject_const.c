// expect: rejected cerastes-unsupported_expression @ L10
// c-only: array compound literals are C only
// twin: tests/memory/array_literal_decay.c
// Pins: 0063 item 12 (0044 item 7, audit B2: M4.6): a const-qualified array literal (C23 6.5.2.5p7 lets two such literals share one object); the tag is 0044's (to confirm at the joined run: an array of const elements may meet item 2's unsupported_type first)
// Pre-registered from record 0063 item 12 (Part D), before any run: rejected
#include <cerastes.h>

int f(void)
{
  const int *p = (const int[]){1, 2, 3};  // <- here
  return p[0];
}
