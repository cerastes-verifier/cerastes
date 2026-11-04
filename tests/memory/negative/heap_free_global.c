// expect: error invalid-free @ L13
// mode: c23, c++20
// twin: tests/memory/heap_free_kinds.c
// Pins: 0047 item 6: the address of a global is no allocation: main holds g whole (0041 program_start), so only the token row fails
// Pre-registered from record 0047 item 6: invalid-free at the call, one error (main holds the global's cell and liveness whole, the token is the one row that fails)
#include <cerastes.h>
#include <stdlib.h>

static int g;

int main(void)
{
  free(&g);  // <- here
  return 0;
}
