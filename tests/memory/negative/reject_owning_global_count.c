// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 (one pin per rejected shape; the encoder reports it at the declaration carrying the attribute): owning(n), a count, on a global (lifted by M4.5e2: owning(n) on a global, record 0060 items 1 and 13, flip B: the message names M4.5e2, not e1b)
// Pre-registered from record 0054 item 5: rejected unsupported_annotation at the declaration
#include <cerastes.h>
#include <stdlib.h>

[[cerastes::owning(2)]] int *g = NULL;  // <- here

int main(void)
{
  return 0;
}
