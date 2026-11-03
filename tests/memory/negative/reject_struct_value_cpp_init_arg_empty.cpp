// expect: rejected cerastes-unsupported_expression @ L23
// twin: tests/memory/struct_value_cpp_nested.cpp
// Pins: 0044 item 6 (re-audit 2; M8): int r = get(T{}); a struct-valued argument inside the initializer of a local: EDG folds the list there and drops its brace flags, so it cannot be told from brace elision (the statement form is struct_value_cpp_nested.cpp)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct T {
  S s;
  int z;
};

int get(T t)
{
  return t.z;
}

int use()
{
  int r = get(T{});  // <- here
  return r;
}
