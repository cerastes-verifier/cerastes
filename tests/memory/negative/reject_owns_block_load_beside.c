// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/owns_block_load_given.c
// Pins: 0054 item 2 ("The place") and item 10: owns(**pp) and owns_block(*pp) name one object twice, so they stay "beside" (0047 item 4) and are rejected; owns(*pp) && owns_block(*pp), which name the pointer cell and the block, are accepted (owns_block_load_given.c)
// Pre-registered from record 0054 item 2 ("The place": whole keys, not roots, are compared): rejected unsupported_annotation at the clause
#include <cerastes.h>

void f(int **pp) CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_owns_block(*pp));  // <- here
