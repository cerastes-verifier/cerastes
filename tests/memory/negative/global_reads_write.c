// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/global_reads_frame.c
// Pins: 0041 item 7: reads(g) is a fraction of g's cell: writing g needs the whole permission
#include <cerastes.h>

static int g = 3;

void poke(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(cerastes_reads(g))
{
  g = 5;
}

int main(void)
{
  return g - g;
}
