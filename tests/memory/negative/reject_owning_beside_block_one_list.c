// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/owning_beside_block.c
// Pins: 0052 item 6c: a block site in one list only beside owning stays rejected (the owning parameter's other list has the owns atom; the two sites would disagree)
// Pre-registered from record 0052 item 6c: rejected unsupported_annotation at the declaration (tag and line guessed)
#include <cerastes.h>

void f(CERASTES_OWNING int *p) CERASTES_REQUIRES(cerastes_owns_block(p));  // <- here
