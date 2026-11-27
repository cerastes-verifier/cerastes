// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 13 (question 320: the program of the retired pin array_reject_pointer_to_array.c, w2, is a positive; P84 (b): the typedef'd spellings): the five spellings of a pointer to a row as a parameter, int a[2][3], int (*a)[3], row *a, row a[] and row a[2] with typedef int row[3], share one adjusted type (sizeof of the two pointer spellings, int (*a)[3] and row *a, is the size of a pointer, as the compilers have it: sizeof_array_argument is for the array spellings only) and each owns the rows of a range and writes its own cell, and the caller reads that cell back right after the call through the matrix (a cell an owns_range callee's ensures does not state is unknown after the call, so no later call is assumed to keep it); sizeof(a[0]) in CODE is the size of a row, 12, in all six spellings (record item 13; in a contract it is rejected: negative/array_rows_reject_forms.c)
// Pre-registered from record 0064 item 13 (Part E), before any run: verifies
#include <cerastes.h>

typedef int row[3];

void w1(int a[2][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[0][0] == 1)
{
  (void)n;
  a[0][0] = 1;
}

void w2(int (*a)[3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[0][1] == 2)
{
  (void)n;
  a[0][1] = 2;
}

void w3(row *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[0][2] == 3)
{
  (void)n;
  a[0][2] = 3;
}

void w4(row a[], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[1][0] == 4)
{
  (void)n;
  a[1][0] = 4;
}

void w5(row a[2], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[1][1] == 5)
{
  (void)n;
  a[1][1] = 5;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  w1(m, 2);
  CERASTES_CHECK(m[0][0] == 1);
  w2(m, 2);
  CERASTES_CHECK(m[0][1] == 2);
  w3(m, 2);
  CERASTES_CHECK(m[0][2] == 3);
  w4(m, 2);
  CERASTES_CHECK(m[1][0] == 4);
  w5(m, 2);
  CERASTES_CHECK(m[1][1] == 5);
  return 0;
}

int size_star(int (*a)[3])
    CERASTES_ENSURES(r, r == 8)
{
  return (int)sizeof(a);
}

int size_row(row *a)
    CERASTES_ENSURES(r, r == 8)
{
  return (int)sizeof(a);
}

int sz1(int a[][3])
    CERASTES_ENSURES(r, r == 12)
{
  return (int)sizeof(a[0]);
}

int sz2(int a[2][3])
    CERASTES_ENSURES(r, r == 12)
{
  return (int)sizeof(a[0]);
}

int sz3(int (*a)[3])
    CERASTES_ENSURES(r, r == 12)
{
  return (int)sizeof(a[0]);
}

int sz4(row *a)
    CERASTES_ENSURES(r, r == 12)
{
  return (int)sizeof(a[0]);
}

int sz5(row a[])
    CERASTES_ENSURES(r, r == 12)
{
  return (int)sizeof(a[0]);
}

int sz6(row a[2])
    CERASTES_ENSURES(r, r == 12)
{
  return (int)sizeof(a[0]);
}
