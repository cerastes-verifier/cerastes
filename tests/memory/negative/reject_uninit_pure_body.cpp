// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c++20
// twin: tests/memory/init_uninit_local.c
// Pins: 0042 item 4 (audit S5): uninit on a local of a pure function's body (as static locals there are, 0041 item 12)
#include <cerastes.h>

CERASTES_PURE int pick(int a)
{
  CERASTES_UNINIT int x;  // <- here
  x = a;
  return x;
}

int main()
{
  return 0;
}
