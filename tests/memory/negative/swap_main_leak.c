// expect: error leak @ L40
// mode: c23, c++20
// twin: tests/memory/verifast/swap_main.c
// Pins: 0047 item 7 and the port of VeriFast's swap.c: main returns holding the point (VeriFast: the leak check at the end of main)
// Pre-registered from record 0047 item 7 (question 123): leak at main's closing brace; re-registered after audit round 1 (record 0047 item 7): closing brace, not the return
#include <cerastes.h>
#include <stdlib.h>

void swap(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*b) && *b == cerastes_old(*a))
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

struct point {
    int x;
    int y;
};

void point_mirror(struct point *p)
    CERASTES_REQUIRES(cerastes_owns(p->x) && cerastes_owns(p->y))
    CERASTES_ENSURES(cerastes_owns(p->x) && cerastes_owns(p->y) && p->x == cerastes_old(p->y) && p->y == cerastes_old(p->x))
{
    swap(&p->x, &p->y);
}

int main(void)
{
    struct point *p = (struct point *)malloc(sizeof(struct point));
    if (p == NULL) { abort(); }
    p->x = 3;
    p->y = 10;
    point_mirror(p);
    bool b = p->x == 10 && p->y == 3;
    CERASTES_CHECK(b);
    return b ? 0 : 1;
}  // <- here
