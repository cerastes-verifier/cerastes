// expect: rejected cerastes-unsupported_expression @ L18
// expect: rejected cerastes-unsupported_expression @ L23
// expect: rejected cerastes-unsupported_expression @ L24
// expect: rejected cerastes-array_address_null_test @ L25
// mode: c23, c++20
// twin: tests/memory/struct_array_member_declared.c
// Pins: 0063 items 1, 5 and 9 (question 292, audit S7 (b)): the decay of a structure global's array member as a global initializer (M4.5e2), malloc of a structure with an array member (M4.5e1e2), the address of the whole member (a pointer to an array: M8) and a null test of the member's address (like a local array's). Tags and lines are to confirm at the joined run (edg-backend's plan C4 and C8; the global initializer may be a different diagnostic in one mode)
// Pre-registered from record 0063 item 1's table (Part C), before any run: rejected
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct S sg = {{1, 2, 3, 4}, 5};
int *gp = sg.a;  // <- here

int f(struct S *q)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct S *p = (struct S *)malloc(sizeof(struct S));  // <- here
  (void)&s.a;  // <- here
  if (s.a == NULL) {  // <- here
    return 1;
  }
  return p == NULL || q == NULL ? s.n : 0;
}
