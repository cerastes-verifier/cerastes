// expect: rejected cerastes-unsupported_expression @ L12
// mode: c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0017 and 0032 item 4: in C++ a const local is a case value, but one whose address is taken is a cell, not a constant: rejected (not lifted before M4.5b2)
#include <cerastes.h>

int case_cell(int a)
{
  const int k = 5;
  const int *q = &k;
  switch (a) {
  case k:
    return *q;
  default:
    return 0;
  }
}
