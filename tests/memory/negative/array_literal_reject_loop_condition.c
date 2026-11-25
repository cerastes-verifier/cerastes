// expect: rejected cerastes-unsupported_expression @ L12
// c-only: array compound literals are C only
// twin: tests/memory/array_literal_decay.c
// Pins: 0063 item 12 (0044 item 7: M8): an array literal in a loop condition is evaluated again without re-entering its block (a loop condition and a do condition both set `in_loop_cond`, so this one pin stands for both: the do-condition pin was dropped on ruling 305; the for step sets its own flag and keeps its own pin, array_literal_reject_for_step.c)
// Pre-registered from record 0063 item 12 (Part D), before any run: rejected
#include <cerastes.h>

int f(void)
{
  int i = 0;
  CERASTES_DECREASES(3 - i)
  while ((int[]){i}[0] < 3) {  // <- here
    i += 1;
  }
  return i;
}
