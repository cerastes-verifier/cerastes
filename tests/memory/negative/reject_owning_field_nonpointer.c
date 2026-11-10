// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 2 (permanent; as reject_owning_nonpointer_local.c): cerastes::owning on a field that is not a pointer
// Pre-registered from record 0055 item 2: rejected unsupported_annotation at the field
#include <cerastes.h>

struct S {
  int n CERASTES_OWNING;  // <- here
  int k;
};

int ident(int a)
{
  return a;
}
