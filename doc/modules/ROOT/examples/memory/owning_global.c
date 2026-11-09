// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pre-registered from record 0054 item 5: an owning static global; install replaces the block it holds (the deep atom comes with owns(g)), current only reads, and main ends while g holds its block (question 174).
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

static int *g CERASTES_OWNING = NULL;

void install(int v)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = (int *)malloc(sizeof(int));
  if (g != NULL) {
    *g = v;
  }
}

int current(void)
    CERASTES_REQUIRES(cerastes_reads(g) && (g == NULL || cerastes_reads(*g)))
    CERASTES_ENSURES(cerastes_reads(g) && (g == NULL || cerastes_reads(*g)))
{
  return g == NULL ? -1 : *g;
}

int main(void)
{
  install(3);
  install(4);
  return current() < 0;
}
// end::source[]
