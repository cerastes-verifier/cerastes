// expect: rejected cerastes-unsupported_declaration @ L8
// twin: tests/scalar/old_cpp.cpp
// Pins: 0015 item 1: a main declared not to return is rejected
// C++ only: in C23 the front end also warns (#3271, a noreturn function with a non-void result), which a rejected test does not list.
#include <cstdlib>
#include <cerastes.h>

[[noreturn]] int main(void)
{
  std::exit(0);
}
