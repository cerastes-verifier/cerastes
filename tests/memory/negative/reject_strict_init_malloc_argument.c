// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/heap_strict_init_free.c
// Pins: 0047 item 8: under --strict-init a call of malloc passed as an argument is rejected, naming d2b
// Pre-registered from record 0047 item 8: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue); re-registered after audit round 1 (record 0047 item 8): audit S6: wherever it stands
#include <cerastes.h>
#include <stdlib.h>

void take(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p));

void f(void)
{
  take((int *)malloc(sizeof(int)));  // <- here
}
