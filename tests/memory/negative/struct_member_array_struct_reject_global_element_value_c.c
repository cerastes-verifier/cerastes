// expect: rejected cerastes-unsupported_expression @ L25
// c-only: in C the front end folds the address of the element of a global structure's member array to a constant and the encoder rejects the by-value pass; C++ accepts it (tests/memory/struct_member_array_struct_global_element_value.cpp)
// twin: tests/memory/struct_member_array_struct_init.c
// Pins: 0064 item 12 and question 346 (audit round C2, finding 4 and the second audit's finding 3; fail closed): an element of the member array of a GLOBAL structure passed by value (`fp(gt.v[1])`) is rejected as `unsupported_expression` in C (the message names a struct object reached through a pointer read from memory, which gt.v[1] is not: the wording is what question 346 leaves), where `fp(t.v[1])` of a local (the twin's elem_pass) and `fp(g[1])` of a global array of structures are accepted. C++ accepts the call and verifies it (the C++ positive, with its twin negative/struct_member_array_struct_global_element_value_wrong.cpp)
// Registered from audit-C2 probe g02b (measured on snapshot a044aff629b8 in C: rejected at the call in main, unsupported_expression, exit 2) and the first run on j3e (this program in C++ is accepted: no row), before the first run of this file; the row's line is inferred from this layout
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

static struct T gt = {{{1, 2}, {3, 4}}, 5};

int fp(struct P a)
    CERASTES_ENSURES(r, r == a.x);

int main(void)
{
  int b = fp(gt.v[1]);  // <- here
  return b;
}
