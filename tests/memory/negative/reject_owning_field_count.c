// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 2 (as reject_owning_global_count.c): owning(n), a count, on a field (re-expected by M4.5e1)
// Pre-registered from record 0055 item 2: rejected unsupported_annotation at the field
#include <cerastes.h>

struct S {
  [[cerastes::owning(2)]] int *q;  // <- here
  int k;
};

int ident(int a)
{
  return a;
}
