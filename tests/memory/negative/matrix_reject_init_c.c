// expect: rejected cerastes-unsupported_expression @ L12
// expect: rejected cerastes-unsupported_declaration @ L18
// mode: c23
// c-only: an array designator is C (in C++ it is the front end's c99_designator error)
// twin: tests/memory/matrix_init.c
// Pins: 0064 items 6 and 16, M8 (array designators): in C a list that elides the braces of the rows ({1, 2, 3, 4, 5, 6} over int m[2][3]) is rejected like the compilers' -Wmissing-braces (the tag of the elision rows of array_struct_reject_init_elision.c), and an array designator over the rows ({[1] = {4, 5, 6}}) is rejected as over an array of scalars (array_reject_designator.c: EDG keeps only the folded index outside templates)
// Pre-registered from record 0064 items 6 and 16 (Part E), before any run: rejected (tags by array_struct_reject_init_elision.c and array_struct_reject_init_designator.c, to confirm at the joined run)
#include <cerastes.h>

int elided(void)
{
  int m[2][3] = {1, 2, 3, 4, 5, 6};  // <- here
  return m[1][2];
}

int designated(void)
{
  int m[2][3] = {[1] = {4, 5, 6}};  // <- here
  return m[1][2];
}
