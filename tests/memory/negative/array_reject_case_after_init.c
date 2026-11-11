// expect: rejected cerastes-unsupported_statement @ L13
// expect: rejected cerastes-unsupported_expression @ L15
// expect: frontend-warning initialization_not_reachable @ L13
// mode: c23, c++20
// twin: tests/memory/array_case_after_plain.c
// Pins: 0059 item 6 and 0012: a declaration with an initializer before the first case label is a statement that no path reaches: rejected as for a scalar (x01: the same two errors); the use after the label is reported too, with the declaration's cascade
// Pre-registered from the M4.5e1a sweep (report 5, s13): the declaration's statement, its cascade at the use, and EDG's #185
#include <cerastes.h>

int pick(int c)
{
  switch (c) {
    int a[2] = {1, 2};
  case 1:
    return a[0];
  default:
    return 0;
  }
}
