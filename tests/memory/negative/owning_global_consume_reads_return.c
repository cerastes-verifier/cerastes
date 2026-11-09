// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 and question 182 (7): a consumer of the block through reads(g) && owns_block(g) frees it and cannot set the global (it holds only a read fraction of g's cell), so after drop() the owning global is non-null and holds no block; f, whose contract carries the deep atom in both lists, returns like that: "returns while owning global g does not hold its block: set it to NULL after freeing it"
// Pre-registered from record 0054 item 5 ("Returns") and owner question 182 (7): permission at f's return, the closing brace of this void function (the deep atom's return row)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void drop(void)
    CERASTES_REQUIRES(cerastes_reads(g) && cerastes_owns_block(g))
    CERASTES_ENSURES(cerastes_reads(g))
{
  free(g);
}

void f(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  if (g != NULL) {
    drop();
  }
}  // <- here

int main(void)
{
  f();
  return 0;
}
