// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/heap_consume.c
// Pins: 0047 item 4: owns_block of a pointer to const: 0040 item 2 rejects owns of a const place, and such a block could never be freed without a cast that drops the qualifier (audit N12)
// Pre-registered from record 0047 item 4: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue); re-registered after audit round 1 (record 0047 item 4): audit N12
#include <cerastes.h>

void f(const int *p) CERASTES_REQUIRES(cerastes_owns_block(p));  // <- here
