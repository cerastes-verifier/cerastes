// expect: error check @ L19
// mode: c23, c++20
// twin: tests/memory/struct_array_global.c
// (a comment line keeps the line numbers of the program this test replaces)
// Pins: 0063 item 9: a structure global of internal linkage has its initial values (g.a[2] is 3, not 4): the check fails (the positive has external linkage, whose values C++ does not know, so the twin is static)
// Pre-registered from record 0063 item 9 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

static struct S g = {{1, 2, 3, 4}, 5};

int main(void)
{
  CERASTES_CHECK(g.a[2] == 4);  // <- here
  return g.n;
}
