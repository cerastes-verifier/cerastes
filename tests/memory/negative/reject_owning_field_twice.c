// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 2 (as reject_owning_global_twice.c): the attribute twice on one field, in both positions
// Pre-registered from record 0055 item 2: rejected unsupported_annotation at the field
#include <cerastes.h>

struct S {
  CERASTES_OWNING int *q CERASTES_OWNING;  // <- here
  int k;
};

int ident(int a)
{
  return a;
}
