// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c++20
// twin: tests/memory/global_declared_unused.c
// Pins: 0041 item 2: a C++ variable template is a template, not a global (M5)
#include <cerastes.h>

template <class T> T vt = T(1);

int ident(int a)
{
  return a;
}
