// expect: rejected cerastes-unsequenced_modification @ L12
// mode: c23, c++20
// twin: tests/memory/array_unsequenced_distinct.c
// Pins: 0059 item 9: the shape names one element twice (or the local i): rejected like the compilers (GCC -Wsequence-point at -Wall in both modes), not left to the path-precise row
// Pre-registered from record 0059 item 9
#include <cerastes.h>

int f(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
{
  int a[3] = {1, 2, 3};
  return a[i]++ + a[i];
}
