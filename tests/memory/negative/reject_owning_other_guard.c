// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 3 (question 75): an owns(*p) under another guard beside owning is rejected (never lifted)
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING int *p, bool c) CERASTES_REQUIRES(c || cerastes_owns(*p)) CERASTES_ENSURES(c || cerastes_owns(*p)) { (void)p; (void)c; }
