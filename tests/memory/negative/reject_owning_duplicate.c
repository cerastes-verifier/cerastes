// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 3: the attribute twice on one parameter is rejected (never lifted)
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING int *p CERASTES_OWNING) { (void)p; }
