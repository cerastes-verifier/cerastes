// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 2 (as reject_owning_const_int.c for a global): cerastes::owning on a pointer to const: an owner of what it may not write
// Pre-registered from record 0055 item 2: rejected unsupported_annotation at the field
#include <cerastes.h>

struct S {
  const int *q CERASTES_OWNING;  // <- here
  int k;
};

int ident(int a)
{
  return a;
}
