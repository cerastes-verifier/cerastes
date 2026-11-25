// expect: rejected cerastes-return_local_address @ L11
// expect: frontend-warning returning_ptr_to_local_temp @ L11
// c-only: array compound literals are C only
// twin: tests/memory/array_literal_decay.c
// Pins: 0063 item 12 (0044 item 7, 0039 item 6's R14): return of an array literal's decay is the address of a block-scope array; EDG may add a frontend-warning returning_ptr_to_local_temp as it does for return &(int){5} (to confirm at the joined run: the tag and the warning line)
// Pre-registered from record 0063 item 12 (Part D), before any run: rejected
#include <cerastes.h>

int *f(void)
{
  return (int[]){1, 2, 3};  // <- here
}
