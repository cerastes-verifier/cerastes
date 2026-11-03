// expect: rejected cerastes-unsupported_declaration @ L13
// expect: rejected cerastes-unsupported_expression @ L15
// mode: c++20
// twin: tests/memory/struct_global_cpp.cpp
// Pins: 0044 item 8 (audit N4): a constexpr function is rejected at its definition, and the element that calls it (a folded constant) in the initializer of a struct global at its own line
#include <cerastes.h>

struct S {
  int x;
  int y;
};

constexpr int two() { return 2; }

static S d = {two(), 1};

int main()
{
  return d.x;
}
