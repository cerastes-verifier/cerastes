// expect: rejected cerastes-unsupported_declaration @ L8
// expect: rejected cerastes-unsupported_declaration @ L13
// mode: c23, c++20
// twin: tests/memory/global_declared_unused.c
// Pins: 0041 item 2: a struct global is rejected until M4.5c (two diagnostics: at the definition of the struct, "a class, struct or union", and at the global, "the type")
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
