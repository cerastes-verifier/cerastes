// expect: rejected cerastes-return_local_address @ L20
// expect: rejected cerastes-return_local_address @ L26
// expect: frontend-warning returning_ptr_to_local_variable @ L20
// expect: frontend-warning returning_ptr_to_local_variable @ L26
// mode: c23, c++20
// twin: tests/memory/struct_array_address.c
// Pins: 0063 item 5 (0039's R14, as 0059 item 10 for a local array): return &s.a[0]; and return s.a; give back the address of a slot of a local structure: rejected like return &x; (EDG's own warning #1056 listed; to confirm at the joined run that EDG warns on both lines)
// Pre-registered from record 0063 item 5 (Part C), before any run: rejected (tags from edg-backend's plan C2)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int *first(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  return &s.a[0];  // <- here
}

int *whole(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  return s.a;  // <- here
}
