// expect: verified
// expect: native-ok
// mode: c23, c++20
// (a comment line keeps the line numbers of the program this test replaces)
// Pins: 0063 item 9 (flip A of M4.5e1e1, the staging pin struct_array_reject_global.c retired; cut 6 not taken): a structure global with an array member and an initializer, read in main: the program_start row group has the array leaf's lines and the facts the member's length; unchanged, line for line
// Pre-registered from record 0063 item 9 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

struct S g = {{1, 2, 3, 4}, 5};

int main(void)
{
  return g.n;
}
