// expect: rejected cerastes-unsupported_expression @ L21
// expect: rejected cerastes-unsupported_expression @ L27
// mode: c23, c++20
// twin: tests/memory/array_struct_init.c
// Pins: 0064 items 6 and 16: a list that elides the braces of an element ({1, 2, 3, 4} over struct P v[2]) or of a nested structure inside an element ({{1, 2, 3}} over struct Q w[1]) is rejected like the compilers' -Wmissing-braces (told from a written list by explicit_braces_on_aggregate); the tag is that of the elision rows of 0043 item 6 and 0063 item 6, to confirm at the joined run
// Pre-registered from record 0064 items 6 and 16 (Part B), before any run: rejected (tag by the existing elision pins reject_struct_init_elision.c and struct_array_reject_init.c)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct Q {
  struct P t;
  int z;
};

int elided(void)
{
  struct P v[2] = {1, 2, 3, 4};  // <- here
  return v[0].x;
}

int nested(void)
{
  struct Q w[1] = {{1, 2, 3}};  // <- here
  return w[0].z;
}
