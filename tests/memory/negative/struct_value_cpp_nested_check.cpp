// expect: error postcondition @ L25
// twin: tests/memory/struct_value_cpp_nested.cpp
// Pins: 0044 item 6: get(T{}) is 0, so the postcondition r == 1 fails at the return
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
    CERASTES_ENSURES(r, r == t.s.y)
{
  return t.s.y;
}

int use()
    CERASTES_ENSURES(r, r == 1)
{
  return get(T{});  // <- here
}
