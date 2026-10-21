// expect: rejected cerastes-call_without_contract @ L12
// twin: tests/scalar/old_cpp.cpp
// Pins: 0015 item 1: std::terminate is not in the trusted table
// (tag observed with the pre-0015 binary: a function with no body and no contract)
#include <exception>
#include <cerastes.h>

int leave(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  if (x > 5) {
    std::terminate();
  }
  return x;
}
