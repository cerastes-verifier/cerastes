// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/heap_strict_init_free.c
// Pins: 0047 item 8: under --strict-init a call of malloc (a ref_to_uninit result) as a return value is rejected, naming d2b (target v / a marked result: P4222R3, P4416R0)
// Pre-registered from record 0047 item 8: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue); re-registered after audit round 1 (record 0047 item 8): audit S6: wherever it stands
#include <cerastes.h>
#include <stdlib.h>

int *make(void)
{
  return (int *)malloc(sizeof(int));  // <- here
}
