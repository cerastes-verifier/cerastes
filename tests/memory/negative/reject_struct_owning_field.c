// expect: rejected wrong_entity_for_attribute @ L13
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 15, question 91: owning on a field is EDG's own refusal (registration target p); owning fields are M4.5d2
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
