// expect: error out-of-bounds @ L12
// mode: c23
// c-only: array compound literals are C only
// twin: tests/memory/array_compound_literal.c
// Pins: 0063 item 12: the literal (int[]){1, 2, 3} has three elements: i == 3 reaches the subscript (out-of-bounds at the access)
// Pre-registered from record 0063 item 12 (Part D), before any run: the one error
#include <cerastes.h>

int f(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
{
  return (int[]){1, 2, 3}[i];  // <- here
}
