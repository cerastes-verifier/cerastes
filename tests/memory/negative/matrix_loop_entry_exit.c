// expect: error permission @ L37
// expect: error permission @ L51
// expect: error permission @ L68
// expect: error permission @ L83
// expect: error permission @ L98
// expect: error permission @ L109
// expect: error permission @ L127
// mode: c23, c++20
// twin: tests/memory/matrix_loop_reach.c
// Pins: 0064 items 5 and 10 (Part E; audit round E2a, finding 3: the loop's ENTRY and BACK-EDGE rows of a two-binder place were pinned by nothing): a loop whose invariant or body reads a matrix carries the permission to every cell of every row (the automatic rows); a matrix given away BEFORE the loop does not hold it at the loop's entry (`permission` at the `while`: "this loop begins without the permission to every cell of every row of ..."), one given away INSIDE the body does not hold it at the end of an iteration (`permission` at the closing brace of the body: "an iteration of this loop ends without the permission ... that it began with"). Each for a local matrix, an array-of-rows parameter, a global matrix in `main` and one row of a local matrix; the auditor's l07e, la1, la2, la3, la4 and la6. Twin: matrix_loop_reach.c and matrix_loop_param.c (the same loops with nothing given away)
// Pre-registered from record 0064 items 5, 9 and 10 (Part E; audit round E2a, finding 3), before any run: permission at L37; permission at L51; permission at L68; permission at L83; permission at L98; permission at L109; permission at L127; corrected once after the first run on j4a (question 160): main held both global loops and Silicon stops a path at its first failing row (the entry row of the h loop, L109), so the g loop's back-edge row (registered L121) was never reached; the h loop moved to a function of its own (global_h, L109 unchanged) and the g loop's closing brace is now L127
#include <cerastes.h>

void consume(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0);

void consume_row(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0);

static int g[2][3] = {{1, 2, 3}, {4, 5, 6}};
static int h[2][3] = {{1, 2, 3}, {4, 5, 6}};

void keep_g(void)
    CERASTES_REQUIRES(cerastes_owns(g));

void keep_h(void)
    CERASTES_REQUIRES(cerastes_owns(h));

int entry_local(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  consume(m, 2);
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {  // <- here
    s = m[i][0];
    i = i + 1;
  }
  return s;
}

int entry_param(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 <= n && n <= 100)
{
  consume(a, n);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && a[0][0] <= 2147483647)
  CERASTES_DECREASES(n - i)
  while (i < n) {  // <- here
    i = i + 1;
  }
  return 0;
}

int back_edge_local(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    if (i == 0) {
      consume(m, 2);
    }
    i = i + 1;
  }  // <- here
  return 0;
}

int back_edge_param(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 <= n && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 0) {
      consume(a, n);
    }
    i = i + 1;
  }  // <- here
  return 0;
}

int back_edge_one_row(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    if (i == 0) {
      consume_row(m[1], 3);
    }
    i = i + 1;
  }  // <- here
  return 0;
}

int global_h(void)
    CERASTES_REQUIRES(cerastes_owns(h))
{
  int s = 0, i = 0;
  keep_h();
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {  // <- here
    s = h[i][0];
    i = i + 1;
  }
  return s;
}

int main(void)
{
  int s = g[0][0] * 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    if (i == 0) {
      keep_g();
    }
    i = i + 1;
  }  // <- here
  return s;
}
