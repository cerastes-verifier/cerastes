// expect: error out-of-bounds @ L19
// mode: c23, c++20
// twin: tests/memory/struct_array_local.c
// (a comment line keeps the line numbers of the program this test replaces)
// Pins: 0063 item 5: i == 4 is past the four elements of s.a: out-of-bounds at the access (the staging pin's program with i <= 4)
// Pre-registered from record 0063 item 5 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(int i)
    CERASTES_REQUIRES(0 <= i && i <= 4)
{
  struct S s = {{1, 2, 3, 4}, 5};
  return s.a[i];  // <- here
}
