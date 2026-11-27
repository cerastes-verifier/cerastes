// expect: error division-by-zero @ L20
// expect: error division-by-zero @ L29
// expect: error division-by-zero @ L38
// expect: error division-by-zero @ L47
// expect: error division-by-zero @ L57
// mode: c23, c++20
// twin: tests/memory/range_struct_wf.c
// Pins: 0064 item 7 (audit round C1, finding 5; f__wf's ghosts, one per leaf path and slot, shared by no two): each function proves p[0].x != 0 in its requires and divides by a different value in its ensures, so the division is by zero in the ensures, once per function. other_leaf: p[0].y, another leaf of the same element (w01); other_slot: p[1].x, the same leaf of another element (w02); owned: under owns_range the body may change p[0].x, so the ensures' value is not the requires' (w03); other_param: q[0].x of another parameter's range (w08); other_index: p[j].x against p[i].x (w11). A shared ghost name would prove each of these (a contract that divides by zero)
// Pre-registered from record 0064 item 7 and the audit's finding 5 (w01, w02, w03, w08, w11; control w04), before any run: division-by-zero at the ensures line of each function
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

int other_leaf(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1 && p[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == 0 || r == 10 / p[0].y))
{
  (void)p;
  (void)n;
  return 0;
}

int other_slot(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1 && p[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == 0 || r == 10 / p[1].x))
{
  (void)p;
  (void)n;
  return 0;
}

int owned(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1 && p[0].x != 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && (r == 0 || r == 10 / p[0].x))
{
  (void)n;
  p[0].x = 0;
  return 0;
}

int other_param(struct P *p, struct P *q, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads_range(q, n) && n > 1 && p[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads_range(q, n) && (r == 0 || r == 10 / q[0].x))
{
  (void)p;
  (void)q;
  (void)n;
  return 0;
}

int other_index(struct P *p, int n, int i, int j)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && 0 <= i && i < n && 0 <= j && j < n && p[i].x != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == 0 || r == 10 / p[j].x))
{
  (void)p;
  (void)n;
  (void)i;
  (void)j;
  return 0;
}
