// expect: rejected cerastes-unsupported_type @ L8
// expect: rejected cerastes-unsupported_expression @ L8
// c-only: C converts a void* implicitly; C++ refuses it in the front end
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 items 2, 3: int *q = v from a void* is rejected twice over, by the type and by the conversion (re-expected by M4.5d2)
#include <cerastes.h>

int void_to_int(void) { void *v = 0; int *q = v; return q == 0; }
