// expect: rejected cerastes-unsupported_expression @ L12
// expect: rejected cerastes-unsupported_expression @ L17
// expect: rejected cerastes-unsupported_expression @ L22
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1: p + i, p - q and a relational comparison of pointers stay rejected, each with the text naming M4.5e2 (before this session the text said pointer operations in general) (lifted by M4.5e2)
// Pre-registered from record 0059 item 1
#include <cerastes.h>

int *add(int *p)
{
  return p + 1;
}

long diff(int *p, int *q)
{
  return p - q;
}

int less(int *p, int *q)
{
  return p < q;
}
