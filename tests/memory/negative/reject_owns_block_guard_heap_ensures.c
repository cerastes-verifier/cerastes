// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 item 4: a block site whose guard reads the heap in an ensures: M4.5d2b
// Pre-registered from record 0047 item 4: rejected unsupported_annotation at the marked line; re-registered after audit round 1 (record 0047 item 4): audit round 2 S2: a block site's guard that reads memory, a global or old
#include <cerastes.h>
#include <stddef.h>

int *f(int *q) CERASTES_REQUIRES(cerastes_owns(*q)) CERASTES_ENSURES(r, cerastes_owns(*q) && (*q > 0 ? cerastes_owns_block(r) : r == NULL));  // <- here
