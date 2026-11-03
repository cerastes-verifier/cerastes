// expect: rejected cerastes-unsupported_expression @ L8
// twin: tests/memory/struct_literal_value_cpp.cpp
// Pins: 0044 item 7 (flip of reject_compound_literal_address.c, the c++20 half): &(int){5} in C++
#include <cerastes.h>

int compound_literal()
{
  int *p = &(int){5};  // <- here
  return *p;
}
