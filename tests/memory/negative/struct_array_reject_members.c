// expect: rejected cerastes-unsupported_declaration @ L21
// expect: rejected cerastes-unsupported_declaration @ L26
// expect: rejected cerastes-unsupported_declaration @ L31
// expect: rejected cerastes-unsupported_declaration @ L36
// expect: rejected cerastes-unsupported_declaration @ L41
// expect: rejected cerastes-unsupported_annotation @ L46
// expect: rejected cerastes-unsupported_annotation @ L51
// expect: rejected cerastes-unsupported_declaration @ L56
// mode: c23, c++20
// twin: tests/memory/struct_array_member_declared.c
// Pins: 0063 items 1 and 2: array members that stay rejected, at the member's declaration: beside an owning member (question 293: the closure of an owning field over an array member is M4.5e1e2), a const array (M4.6), a volatile element (M8), an array of structures and an array of arrays (M4.5e1e2), a zero-length member (M8), and owning and ref_to_uninit on an array member itself (M4.8, unsupported_annotation: the two lines the plan's table gives that tag; the structure may print a declaration error too). The structures are used as pointees so that each one is read. Tags, lines and the one-error-per-structure behaviour are to confirm at the joined run (a zero-length member may be the front end's own error: then it moves to a file of its own)
// Pre-registered from record 0063 items 1 and 2 (Part C), before any run: rejected (tags from edg-backend's plan C1)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct OwningBeside {
  int a[4];  // <- here
  int *p CERASTES_OWNING;
};

struct ConstArray {
  const int a[4];  // <- here
  int n;
};

struct VolatileArray {
  volatile int a[4];  // <- here
  int n;
};

struct StructElements {
  struct P v[2];  // <- here
  int n;
};

struct ArrayOfArrays {
  int m[2][3];  // <- here
  int n;
};

struct OwningArray {
  CERASTES_OWNING int a[4];  // <- here
  int n;
};

struct MarkedArray {
  CERASTES_REF_TO_UNINIT int a[4];  // <- here
  int n;
};

struct ZeroLength {
  int z[0];  // <- here
  int n;
};

void use(struct OwningBeside *a, struct ConstArray *b, struct VolatileArray *c, struct StructElements *d, struct ArrayOfArrays *e, struct ZeroLength *f, struct OwningArray *g, struct MarkedArray *h)
{
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  (void)g;
  (void)h;
}
