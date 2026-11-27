// expect: rejected cerastes-unsupported_expression @ L27
// expect: rejected cerastes-unsupported_expression @ L33
// mode: c23
// c-only: the C clause of record 0064 item 16; the C++ rows of the same forms are array_struct_reject_designated_zero.cpp
// twin: tests/memory/array_struct_init_zero.c
// Pins: 0064 item 16, "Designated lists": in C a braced designated value whose 0 elides a structure's braces ({.t = {0}} over W, whose T begins with a structure) is the nested list GCC flags at -Wall (the lone {0} is only the whole initializer), and a NON-zero unbraced designated structure member ({.p = 1}) is flagged too; the unbraced designated ZERO ({.p = 0}) stays accepted (array_struct_init_zero.c). The frozen build accepted both rows
// Pre-registered from record 0064 item 16, "Designated lists", before any run: rejected (tag as array_struct_reject_nested_zero.c)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P p;
  int z;
};

struct W {
  struct T t;
  int k;
};

int braced(void)
{
  struct W w = {.t = {0}};  // <- here
  return w.k;
}

int nonzero(void)
{
  struct T t = {.p = 1};  // <- here
  return t.z;
}
