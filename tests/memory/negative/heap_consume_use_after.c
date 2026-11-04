// expect: error permission @ L27
// mode: c23, c++20
// twin: tests/memory/heap_consume.c
// Pins: 0047 item 4: the callee took the block, so the caller's later access is `permission`
// Pre-registered from record 0047 item 4: permission at the access after the call
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

void dispose(struct account *a)
    CERASTES_REQUIRES(cerastes_owns_block(a))
{
  free(a);
}

int make_and_dispose(void)
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a == NULL) {
    return -1;
  }
  a->balance = 5;
  dispose(a);
  return a->balance;  // <- here
}
