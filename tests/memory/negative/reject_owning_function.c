// expect: error lifetime @ L8
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0047 item 5 (flipped by M4.5d2a, owner question 124): [[cerastes::owning]] on a function is ensures(r == NULL || owns_block(r)); p has no contract, so returning it fails the liveness use check (0028 item 3a, `lifetime`) before the missing token could fail the return row; heap_owning_static.c pins the `permission` case (a pointer the function can name but does not own)
#include <cerastes.h>
#include <stddef.h>

CERASTES_OWNING int *f(int *p) { return p; }  // <- here
