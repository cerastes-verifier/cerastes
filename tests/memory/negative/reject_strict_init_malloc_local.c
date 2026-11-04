// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/heap_strict_init_free.c
// Pins: 0047 item 8: under --strict-init a local initialized from a ref_to_uninit result (malloc's) must itself be marked ref_to_uninit (target v: P4222R3 and P4416R0, M4.5d2b)
// Pre-registered from record 0047 item 8: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue); re-registered after audit round 1 (record 0047 item 8): every call of a ref_to_uninit-result function is rejected, wherever it stands (audit S6); the line is the call
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  int *p = (int *)malloc(sizeof(int));  // <- here
  free(p);
  return 0;
}
