// expect: rejected cerastes-unsupported_declaration @ L12
// expect: rejected cerastes-unsupported_declaration @ L13
// expect: frontend-warning declared_but_not_referenced @ L12
// expect: frontend-warning declared_but_not_referenced @ L13
// mode: c23, c++20
// twin: tests/memory/global_array_declared.c
// Pins: 0063 items 1 and 9 (M4.5e2): the address of an element as a global initializer (&g[2]: a subobject path, not the decay) and g + 1 (a folded address with an offset) are rejected like &h.x (0044 item 8); only the decay int *gp = g; is accepted (global_array_escape.cpp)
// Pre-registered from record 0063 item 2 (Part B), before any run: rejected
#include <cerastes.h>

static int base[4];
static int *addr = &base[2];  // <- here
static int *plus = base + 1;  // <- here

int ident(int a)
{
  return a;
}
