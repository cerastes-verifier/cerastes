// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/memory/global_declared_unused.c
// Pins: 0041 item 2: a thread-local global has a copy per thread: rejected (M8)
#include <cerastes.h>

thread_local int tls = 0;

int ident(int a)
{
  return a;
}
