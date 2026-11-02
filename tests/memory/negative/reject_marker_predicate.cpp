// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c++20
// twin: tests/memory/init_cpp.cpp
// Pins: 0042 item 4 (audit S5): a marker on a parameter of a predicate
#include <cerastes.h>

CERASTES_PREDICATE bool valid(int *p CERASTES_REF_TO_UNINIT) { return p != nullptr; }  // <- here

int main()
{
  return 0;
}
