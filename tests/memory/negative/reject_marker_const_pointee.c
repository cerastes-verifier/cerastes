// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 4: a marker on a pointer to a const object: nothing could initialize it (M4.6 lifts const objects as cells, not this)
#include <cerastes.h>

void f(const int *p CERASTES_REF_TO_UNINIT) { (void)p; }  // <- here
