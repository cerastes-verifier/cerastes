// expect: rejected cerastes-unsupported_statement @ L10
// expect: frontend-warning declared_but_not_referenced @ L10
// mode: c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0032 item 4: a cell declared as the body of an if (no block, so no closing brace to leave its scope at) is rejected (not lifted before b1c)
#include <cerastes.h>

int if_declaration(int c)
{
  if (c) int x = 5, *p = &x;
  return 0;
}
