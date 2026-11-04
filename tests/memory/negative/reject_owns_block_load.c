// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/heap_consume.c
// Pins: 0047 item 4: the `owns_block` forms of item 4 that stay rejected: an operand reached through a load (d2b)
// Pre-registered from record 0047 item 4: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue)
#include <cerastes.h>

void f(int **pp) CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp));  // <- here
