// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/heap_consume.c
// Pins: 0047 item 4: a block site whose guard reads the heap (`*q > 0 ? owns_block(p) : true`) in a requires: a guard over the heap is M4.5d2b
// Pre-registered from record 0047 item 4: rejected unsupported_annotation at the marked line; re-registered after audit round 1 (record 0047 item 4): audit round 2 S2: a block site's guard that reads memory, a global or old
#include <cerastes.h>
#include <stddef.h>

void f(int *q, int *p) CERASTES_REQUIRES(cerastes_owns(*q) && (*q > 0 ? cerastes_owns_block(p) : true));  // <- here
