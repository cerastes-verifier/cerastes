// expect: error invalid-free @ L12
// mode: c23, c++20
// twin: tests/memory/heap_free_kinds.c
// Pins: 0047 item 6 (twin of free's contract, 0046 item 9): the address of a local has no token: only the token row fails (the local's liveness and cell are whole)
// Pre-registered from record 0047 item 6 (token held at write and true): invalid-free at the call, one error
#include <cerastes.h>
#include <stdlib.h>

int free_local(void)
{
  int x = 0;
  free(&x);  // <- here
  return x;
}
