// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 4: a marker on a pointer to a function (M4.7 owns function pointers)
#include <cerastes.h>

void f(CERASTES_REF_TO_UNINIT void (*fp)(void)) { (void)fp; }  // <- here
