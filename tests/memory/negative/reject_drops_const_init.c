// expect: rejected bad_initializer_type @ L11
// mode: c23, c++20
// twin: tests/memory/const_pointer.c
// Pins: 0032 item 3: C only warns (#144) about an initialization that drops const (C++ refuses it), verify mode makes it an error
#include <cerastes.h>

int drop_const(void)
{
  const int k = 1;
  const int *pc = &k;
  int *p = pc;
  return p == 0;
}
