// expect: error leak @ L13
// mode: c23, c++20
// twin: tests/memory/block_guard_heap_requires.c
// Pins: 0052 item 6b: the callee takes the block under the heap guard and neither frees nor returns it: leak at its closing brace
// Pre-registered from record 0052 item 6b: leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

void maybe_free(int *q, int *p)
    CERASTES_REQUIRES(cerastes_owns(*q) && (*q > 0 ? cerastes_owns_block(p) : true))
    CERASTES_ENSURES(cerastes_owns(*q) && *q == cerastes_old(*q))
{
}  // <- here
