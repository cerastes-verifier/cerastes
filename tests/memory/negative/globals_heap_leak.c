// expect: error leak @ L39
// mode: c23, c++20
// twin: tests/memory/verifast/globals_heap.c
// Pins: 0047 item 7 and the port of VeriFast's globals.c: main returns without freeing ctr (VeriFast: the leak check at the end of main)
// Pre-registered from record 0047 item 7 (question 123): leak at main's closing brace; re-registered after audit round 1 (record 0047 item 7): closing brace, not the return
#include <cerastes.h>
#include <stdlib.h>

static int x;

struct counter {
    int f;
};

static struct counter *c;

void m(void)
    CERASTES_REQUIRES(cerastes_owns(x) && x == 7 && cerastes_owns(c) && cerastes_owns(c->f) && c->f < 1000)
    CERASTES_ENSURES(cerastes_owns(x) && x == 8 && cerastes_owns(c) && c == cerastes_old(c) && cerastes_owns(c->f) && c->f == cerastes_old(c->f) + 1)
{
    int y = x;
    x = y + 1;
    c->f = c->f + 1;
}

int main(void)
{
    int x0 = x;
    CERASTES_CHECK(x0 == 0);
    x = 7;
    struct counter *ctr = (struct counter *)malloc(sizeof(struct counter));
    if (ctr == NULL) abort();
    ctr->f = 42;
    c = ctr;
    m();
    int ctr_f = ctr->f;
    CERASTES_CHECK(ctr_f == 43);
    return x0 + ctr_f - 43;
}  // <- here
