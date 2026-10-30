// expect: rejected cerastes-annotation_mismatch @ L11
// mode: c23, c++20
// twin: tests/memory/owning_typedef.c
// Pins: 0040 item 3 (audit N2): the typedef carries owning and the definition does not: annotation_mismatch at the later declaration
#include <cerastes.h>
#include <stddef.h>

typedef void fn_t(int *p CERASTES_OWNING);
fn_t f;

void f(int *p) { (void)p; }
