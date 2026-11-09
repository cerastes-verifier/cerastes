// expect: rejected wrong_entity_for_attribute @ L13
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 15, question 91, 0054 item 10: owning on a field is EDG's own refusal (registration targets r|v|p, no d); owning fields are M4.5d2b2b
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct N {
  CERASTES_OWNING struct S *p;  // <- here
  int v;
};

int ident(int a)
{
  return a;
}
