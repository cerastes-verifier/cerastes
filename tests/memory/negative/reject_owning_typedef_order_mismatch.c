// expect: rejected cerastes-annotation_mismatch @ L9
// mode: c23, c++20
// twin: tests/memory/owning_typedef.c
// Pins: 0040 item 3 (audit round 2 S2): the definition comes first without owning, then fn_t f; brings the typedef's owning: annotation_mismatch, reported at the declaration without it and naming the one that has it
#include <cerastes.h>
#include <stddef.h>

typedef void fn_t(CERASTES_OWNING int *p);
void f(int *p) { if (p != NULL) *p = 1; }
fn_t f;
