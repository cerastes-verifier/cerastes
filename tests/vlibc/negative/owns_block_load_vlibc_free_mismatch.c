// expect: error invalid-free @ L27
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/owns_block_load_vlibc.c
// Pins: 0054 item 2 (0050 item 3): a vlibc block given through *pp is freed with the platform's free: the token's value says whose block it is, so this is invalid-free
// Pre-registered from record 0054 item 2 and 0050 items 3, 5: invalid-free at the free call (as free_of_vlibc.c)
#include <vlibc/stdlib.h>
#include <cerastes.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_vlibc_block(*pp) && **pp == 5)
{
  *pp = (int *)vlibc_malloc(sizeof(int));
  if (*pp == NULL) {
    abort();
  }
  **pp = 5;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 5)
{
  int *p = NULL;
  make(&p);
  int v = *p;
  free(p);  // <- here
  return v;
}
