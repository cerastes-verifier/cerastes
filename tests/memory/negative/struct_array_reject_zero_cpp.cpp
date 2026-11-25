// expect: rejected cerastes-unsupported_expression @ L15
// mode: c++20
// twin: tests/memory/struct_array_init.c
// Pins: 0063 item 6 (question 303): in C++ {0} whose 0 elides the braces of an array member in a structure of two or more members is rejected like clang++'s -Wmissing-braces (C accepts it, and so do both modes for a structure whose only member is the array and for {{0}, 0})
// Pre-registered from record 0063 item 6 (Part C), before any run: rejected (tag from edg-backend's plan C3; to confirm at the joined run)
#include <cerastes.h>

struct S {
  int a[4];
  int n;
};

int f()
{
  S s = {0};  // <- here
  return s.n;
}
