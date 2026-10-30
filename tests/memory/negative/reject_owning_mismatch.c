// expect: rejected cerastes-annotation_mismatch @ L9
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 3: a declaration with owning and the definition without it: annotation_mismatch, reported at the later declaration (0006 item 3)
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING int *p);
void f(int *p) { (void)p; }
