// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 6 and 0050 item 5 (audit round 3, the result split's cost): eight blocks held in locals and four calls of a bodiless `int *id(int *a)` with `ensures r == a`, every block freed once: a call's pointer result is split against its own pointer arguments only, so the program verifies at once; before that rule it ran over 270 s (the auditor's a2r/stress/st_8_4.c)
// Pre-registered from audit round 3 (a2r/stress/st_8_4.c) and il-analyst's change to the split: verifies in both modes, in seconds (the results are written through so that no local is unused under -Wextra)
#include <stdlib.h>
#include <cerastes.h>

int *id(int *a)
    CERASTES_ENSURES(r, r == a);

int f(void)
{
  int *p0 = (int *)malloc(sizeof(int));
  if (p0 == NULL) {
    return -1;
  }
  int *p1 = (int *)malloc(sizeof(int));
  if (p1 == NULL) {
    free(p0);
    return -1;
  }
  int *p2 = (int *)malloc(sizeof(int));
  if (p2 == NULL) {
    free(p0);
    free(p1);
    return -1;
  }
  int *p3 = (int *)malloc(sizeof(int));
  if (p3 == NULL) {
    free(p0);
    free(p1);
    free(p2);
    return -1;
  }
  int *p4 = (int *)malloc(sizeof(int));
  if (p4 == NULL) {
    free(p0);
    free(p1);
    free(p2);
    free(p3);
    return -1;
  }
  int *p5 = (int *)malloc(sizeof(int));
  if (p5 == NULL) {
    free(p0);
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    return -1;
  }
  int *p6 = (int *)malloc(sizeof(int));
  if (p6 == NULL) {
    free(p0);
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);
    return -1;
  }
  int *p7 = (int *)malloc(sizeof(int));
  if (p7 == NULL) {
    free(p0);
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);
    free(p6);
    return -1;
  }
  int *r0 = id(p0);
  int *r1 = id(p1);
  int *r2 = id(p2);
  int *r3 = id(p3);
  *r0 = 0;
  *r1 = 0;
  *r2 = 0;
  *r3 = 0;
  free(p0);
  free(p1);
  free(p2);
  free(p3);
  free(p4);
  free(p5);
  free(p6);
  free(p7);
  return 0;
}
