// expect: rejected cerastes-unsupported_annotation @ L16
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 2 (question 191): two structures that own each other: the cycle closes at the field of the second structure (B's a), which is where it is reported, naming M4.5f2
// Pre-registered from record 0055 item 2: rejected unsupported_annotation at the field that closes the cycle (derived from the record, not observed: the line of the first field, A's b, is the neighbour)
#include <cerastes.h>

struct B;

struct A {
  struct B *b CERASTES_OWNING;
  int k;
};

struct B {
  struct A *a CERASTES_OWNING;  // <- here
  int m;
};

int ident(int a)
{
  return a;
}
