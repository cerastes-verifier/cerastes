// expect: rejected cerastes-unsupported_annotation @ L8
// c-only: restrict does not exist in C++20 (decision 0037 item 2)
// twin: tests/memory/heap_borrow_then_free.c
// Pins: 0047 item 4: a restrict parameter whose only atom is a block site: 0037 item 2's restrict_backed reads owns atoms only (audit N11; d2b may lift it)
// Pre-registered from record 0047 item 4: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue); re-registered after audit round 1 (record 0047 item 4): audit N11
#include <cerastes.h>

void f(int *restrict p) CERASTES_REQUIRES(cerastes_owns_block(p)) CERASTES_ENSURES(cerastes_owns_block(p));  // <- here
