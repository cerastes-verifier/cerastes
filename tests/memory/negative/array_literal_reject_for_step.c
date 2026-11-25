// expect: rejected cerastes-unsupported_expression @ L12
// c-only: array compound literals are C only
// twin: tests/memory/array_literal_decay.c
// Pins: 0063 item 12 (0044 item 7: M8): an array literal in a for step is evaluated again without re-entering its block; it reaches the shared rejection site (`in_loop_cond || in_for_step`) through its own flag, `in_for_step`, which no other test sets (the loop condition and the do condition set `in_loop_cond`: array_literal_reject_loop_condition.c)
// Pre-registered from record 0063 item 12 (Part D), before any run: rejected
#include <cerastes.h>

int f(void)
{
  int i = 0;
  CERASTES_DECREASES(3 - i)
  for (; i < 3; i = (int[]){i + 1}[0]) {  // <- here
  }
  return i;
}
