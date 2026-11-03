// expect: verified
// expect: native-ok
// mode: c23, c++20
// A structure global: its members start with the initializer's values, a contract names the whole object or one member.
// tag::source[]
#include <cerastes.h>

struct counter {
  int hits;
  int misses;
};

static struct counter stats = {.hits = 0, .misses = 2};

void hit(void)
    CERASTES_REQUIRES(cerastes_owns(stats.hits) && stats.hits < 1000)
    CERASTES_ENSURES(cerastes_owns(stats.hits) && stats.hits == cerastes_old(stats.hits) + 1)
{
  stats.hits += 1;
}

int total(void)
    CERASTES_REQUIRES(cerastes_reads(stats) && 0 <= stats.hits && stats.hits <= 1000 && 0 <= stats.misses && stats.misses <= 1000)
    CERASTES_ENSURES(r, cerastes_reads(stats) && r == stats.hits + stats.misses)
{
  return stats.hits + stats.misses;
}

int main(void)
{
  hit();
  CERASTES_CHECK(stats.hits == 1 && stats.misses == 2);
  return total() == 3 ? 0 : 1;
}
// end::source[]
