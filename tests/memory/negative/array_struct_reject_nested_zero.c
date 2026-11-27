// expect: rejected cerastes-unsupported_expression @ L24
// expect: rejected cerastes-unsupported_expression @ L28
// mode: c23, c++20
// twin: tests/memory/array_struct_init.c
// Pins: 0064 item 16 (question 321): {{0}} whose inner 0 elides the braces of a nested structure inside a member list is rejected in both modes when that structure has two or more members (GCC flags it in C: the member's own list {0} is not the whole initializer; clang++ flags it in C++: the elided P's parent T has two members), as a global and as a local. The frozen build accepted both; no existing file holds the form
// Pre-registered from record 0064 item 16 (Part B), before any run: rejected (tag as array_struct_reject_zero_cpp.cpp)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P p;
  int z;
};

struct W {
  struct T t;
  int k;
};

struct W g = {{0}};  // <- here

int local(void)
{
  struct W w = {{0}};  // <- here
  return w.k;
}
