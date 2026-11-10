// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 2 (question 191): a structure that owns itself through its field is rejected at the field, naming M4.5f2: recursive ownership needs recursive predicates
// Pre-registered from record 0055 item 2: rejected unsupported_annotation at the field that closes the cycle
#include <cerastes.h>

struct node {
  struct node *next CERASTES_OWNING;  // <- here
  int value;
};

int ident(int a)
{
  return a;
}
