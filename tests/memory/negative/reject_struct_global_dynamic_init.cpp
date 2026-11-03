// expect: rejected cerastes-unsupported_declaration @ L18
// twin: tests/memory/struct_global_cpp.cpp
// Pins: 0044 item 8 (0041 item 2): a C++ struct global initialized by a call is initialized dynamically: rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

S mk()
    CERASTES_ENSURES(r, r.x == 1 && r.y == 2)
{
  S s = {1, 2};
  return s;
}

static S d = mk();  // <- here

int main()
{
  return d.x;
}
