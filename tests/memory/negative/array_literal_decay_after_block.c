// expect: error lifetime @ L14
// c-only: array compound literals are C only
// twin: tests/memory/array_literal_decay.c
// Pins: 0063 item 12: a literal of an inner block is exited with its block (0044 item 7's rule for the hidden array): a read q[0] through the pointer after the block is an access to a dead object (the liveness row of the element access, lifetime); the struct form of 0044 reads p->x, which is not a use of p's value and fails as permission
// Pre-registered from record 0063 item 12 (Part D), before any run: the one error (the category is the liveness row's; to confirm at the joined run)
#include <cerastes.h>

int f(void)
{
  int *q = 0;
  {
    q = (int[]){1, 2, 3};
  }
  return q[0];  // <- here
}
