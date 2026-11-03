// expect: rejected cerastes-unsupported_expression @ L23
// twin: tests/memory/struct_value_cpp_nested.cpp
// Pins: 0044 item 6 (audit N7; M8): a C++ designated list that skips a nested struct member, get(T{.z = 1}): C++20 zero-fills the skipped member without braces (P65 H1), the tuple of its leaves is not built from it
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
  return get(T{.z = 1});  // <- here
}
