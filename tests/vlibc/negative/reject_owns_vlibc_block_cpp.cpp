// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c++20
// twin: tests/vlibc/vlibc_owns_vlibc_block.c
// Pins: 0052 item 7 (0050 item 7): the vlibc block spelling is rejected in C++ like vlibc's allocations (until M7)
// Pre-registered from record 0052 item 7: rejected unsupported_annotation at the declaration (tag and line guessed; the C name is the only spelling, record 0052 item 7)
#include <cerastes.h>

void dispose(int *p) CERASTES_REQUIRES(cerastes_owns_vlibc_block(p));  // <- here
