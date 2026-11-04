// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/heap_strict_init_free.c
// Pins: 0047 item 8: under --strict-init a call of malloc stored through a pointer (`*pp = malloc(...)`) is rejected, naming d2b
// Pre-registered from record 0047 item 8: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue); re-registered after audit round 1 (record 0047 item 8): audit S6: wherever it stands
#include <cerastes.h>
#include <stdlib.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  *pp = (int *)malloc(sizeof(int));  // <- here
}
