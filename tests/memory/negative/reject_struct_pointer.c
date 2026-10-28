// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 2: a pointer to a struct (FILE of <stdio.h> is one) is rejected (not lifted by b1b or b1c: re-expected by M4.5c)
#include <cerastes.h>
#include <stdio.h>
int struct_pointer(void) { FILE *p = 0; return p == 0; }
