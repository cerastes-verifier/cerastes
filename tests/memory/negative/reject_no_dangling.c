// expect: rejected unrecognized_attribute @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 5 (question 179): cerastes::no_dangling is not an attribute of the closed namespace in C (M7 reads std::no_dangling as a checked identity): the front end names it unknown
// Pre-registered from record 0058 item 5: rejected unrecognized_attribute at the parameter (EDG's #1097, "unknown attribute"; the diagnostic tag is a guess, as unknown_name_typo.cpp's frontend-error unrecognized_attribute)
#include <cerastes.h>

void f(int *p [[cerastes::no_dangling]]) { (void)p; }  // <- here
