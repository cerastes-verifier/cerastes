// expect: error invalid-free @ L20
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_loop_inside.c
// Pins: 0050 item 5: a vlibc block allocated in the body is freed there with the platform's free
// Pre-registered from record 0050 item 5: invalid-free at the free in the body
#include <vlibc/stdlib.h>
#include <cerastes.h>

int loop_allocates(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int *p = (int *)vlibc_malloc(sizeof(int));
    if (p != NULL) {
      *p = i;
      free(p);  // <- here
    }
    i += 1;
  }
  return 0;
}
