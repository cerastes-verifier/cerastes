// expect: rejected cerastes-unsupported_expression @ L8
// c-only: C converts the pointers implicitly (EDG warns in C23 without --diag_error); C++ refuses it in the front end
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0032 item 3: in C a pointer to an enumeration converted to a pointer to its underlying integer type is rejected (C++ refuses it in the front end)
#include <cerastes.h>

enum E { A, B };
int enum_pointer(void) { enum E e = A; unsigned *pu = &e; return pu == 0; }
