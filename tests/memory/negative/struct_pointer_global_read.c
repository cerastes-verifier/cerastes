// expect: error lifetime @ L17
// mode: c23, c++20
// twin: tests/memory/struct_pointer_global.c
// Pins: 0043 item 8, 0041 item 4: the unknown value of gp is tested: it uses gp's target, which nothing in main holds alive (lifetime)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

struct S *gp;

int main(void)
{
  if (gp == NULL) {  // <- here
    return 0;
  }
  return 1;
}
