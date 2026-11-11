// expect: rejected cerastes-unsupported_declaration @ L15
// expect: rejected cerastes-unsupported_declaration @ L21
// expect: rejected cerastes-unsupported_declaration @ L27
// expect: rejected cerastes-unsupported_declaration @ L33
// expect: rejected cerastes-unsupported_declaration @ L39
// mode: c23
// c-only: in C++ an array designator is c99_designator (the _cxx pins); GNU's range designator is C only
// twin: tests/memory/array_init_partial.c
// Pins: 0059 item 3 (audit round 1, B1 and B2): array designators are rejected in C: outside templates EDG keeps only the folded index, so a wrapping index or bound could not be checked as written (0024); an index given twice and GNU's range designator go with them (lifted by M8, an EDG change: question 69)
// Pre-registered from record 0059 item 3 as updated by audit round 1
#include <cerastes.h>

int one(void)
{
  int a[3] = {[2] = 5};
  return a[2];
}

int wrapped(void)
{
  int b[3] = {[65536u * 65536u + 1] = 7};
  return b[1];
}

int unsized(void)
{
  int c[] = {[65536u * 65536u] = 9};
  return c[0];
}

int twice(void)
{
  int d[2] = {[0] = 1, [0] = 2};
  return d[0];
}

int range(void)
{
  int e[4] = {[1 ... 2] = 5};
  return e[1];
}
