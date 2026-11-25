// expect: rejected cerastes-unsupported_expression @ L36
// expect: rejected cerastes-unsupported_expression @ L42
// mode: c23
// c-only: row 1 is accepted in C++ (clang++ is silent: the elided P is the only member of T1) but cannot be a native-ok positive there (-Wextra's missing-field-initializers on k); row 2 is rejected in C++ today by the elided recursion's read_list, so the file's two rows differ by mode
// twin: tests/memory/array_struct_init_zero.c
// Pins: 0064 item 16 (question 321): in C, {{0}} whose inner 0 elides a subaggregate's braces is rejected like GCC's -Wmissing-braces at -Wall: row 1, a member list {0} over T1 whose first member is a structure (T1 has one member); row 2, T t = {{0}} over a structure that begins with a structure beginning with an array. The frozen build accepted both in C
// Pre-registered from record 0064 item 16 (Part B), before any run: rejected (tag as array_struct_reject_zero_cpp.cpp)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T1 {
  struct P p;
};

struct W1 {
  struct T1 t;
  int k;
};

struct S {
  int a[4];
  int n;
};

struct U {
  struct S s;
  int z;
};

int row1(void)
{
  struct W1 w = {{0}};  // <- here
  return w.k;
}

int row2(void)
{
  struct U u = {{0}};  // <- here
  return u.z;
}
