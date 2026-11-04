// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/heap_consume.c
// Pins: 0047 item 4: the `owns_block` forms of item 4 that stay rejected: a global operand (d2b)
// Pre-registered from record 0047 item 4: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue)
#include <cerastes.h>

static int *gp;

void f(void) CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns_block(gp));  // <- here
