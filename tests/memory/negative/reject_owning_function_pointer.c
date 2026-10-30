// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 3: owning on a pointer to function is rejected (M4.7)
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING void (*fp)(void)) { (void)fp; }
