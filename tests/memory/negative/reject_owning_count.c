// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 3: owning(n) is rejected (re-expected by M4.5e1)
#include <cerastes.h>
#include <stddef.h>

void f([[cerastes::owning(2)]] int *p) { (void)p; }
