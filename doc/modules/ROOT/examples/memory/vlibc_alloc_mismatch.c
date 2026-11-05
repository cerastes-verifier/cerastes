// expect: error invalid-free @ L14
// mode: c23
// c-only: vlibc_malloc creates no C++ objects; cerastes rejects it in C++ (decision 0050 item 7)
// twin: doc/modules/ROOT/examples/memory/vlibc_alloc.c
// tag::source[]
#include <cerastes.h>
#include <vlibc/stdlib.h>

void f(void)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p != NULL) {
    *p = 1;
    free(p);   // a block of vlibc's heap, freed by the platform's free
  }
}
// end::source[]
