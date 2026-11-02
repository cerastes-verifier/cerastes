// (the struct definition at L8 is accepted since record 0043, flip (b) of question 93: no expect line for it)
// expect: rejected cerastes-unsupported_declaration @ L13
// mode: c23, c++20
// twin: tests/memory/global_declared_unused.c
// Pins: 0041 item 2, record 0043 item 15 (flip (b)): the struct definition is accepted; a struct global is rejected, naming M4.5c2 (one diagnostic, at the global)
#include <cerastes.h>

struct point {
  int x;
  int y;
};

struct point origin;

int ident(int a)
{
  return a;
}
