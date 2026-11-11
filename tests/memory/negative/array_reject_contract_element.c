// expect: rejected cerastes-unsupported_expression @ L8
// mode: c23, c++20
// twin: tests/memory/array_ptr_param_first.c
// Pins: 0059 item 1: an element in a contract (p[0] in a requires) is rejected; the text names e1b (lifted by M4.5e1b)
#include <cerastes.h>

int first(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p[0] > 0)
{
  return *p;
}
