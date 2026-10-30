// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 3: owning on void * is rejected (never lifted; the type itself is rejected too if the type check runs first)
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING void *p) { (void)p; }
