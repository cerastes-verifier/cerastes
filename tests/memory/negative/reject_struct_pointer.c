// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0043 item 2 (re-messaged by M4.5c1; was 0032 item 2): a pointer to a structure from a system header (FILE of <stdio.h>) is rejected, as 0018 rejects system enumerations; a pointer to a verified struct is accepted
#include <cerastes.h>
#include <stdio.h>
int struct_pointer(void) { FILE *p = 0; return p == 0; }
