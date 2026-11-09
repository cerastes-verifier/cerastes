// expect: error leak @ L27
// mode: c23, c++20
// twin: tests/memory/owning_loop_freed_alias_call.c
// Pins: 0054 item 3 (d) (S39 w2a): without the free through the alias the owner still holds its block across the loop, carried by the held-guarded carrier, and a's exit row reports it at the return
// Pre-registered from record 0054 item 3 (d): leak at the return statement (owning exit row, 0052 item 3)
#include <cerastes.h>
#include <stdlib.h>

void touch(void) {}

int main(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 0;
  int *al = a;
  *al = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    touch();
    i += 1;
  }
  return 0;  // <- here
}
