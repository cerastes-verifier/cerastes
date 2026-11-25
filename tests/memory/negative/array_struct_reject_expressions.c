// expect: rejected cerastes-array_address_null_test @ L23
// expect: rejected cerastes-array_address_null_test @ L33
// expect: rejected cerastes-array_address_null_test @ L40
// expect: rejected cerastes-array_address_null_test @ L47
// expect: rejected cerastes-array_address_null_test @ L53
// expect: rejected cerastes-unsequenced_modification @ L60
// expect: rejected cerastes-unsequenced_modification @ L68
// mode: c23, c++20
// twin: tests/memory/array_struct_address.c
// Pins: 0064 items 5 and 11 (question 322): a truth test or a null comparison of the address of an element or of a member of an element (if (&v[1]), &v[i].x == NULL, !&v[i], &p[i] != NULL on a struct P *p, and one inside a check) is rejected like the compilers (GCC -Waddress at -Wall on each; clang on &v[i].x), array_address_null_test; and one leaf of a NAMED array named twice in one expression with a write (v[0].x = v[0].x++, v[i].x = v[i].x++) is rejected like GCC's -Wsequence-point, unsequenced_modification, not left to the path-precise row (the pointer form p[i].x = p[i].x++ has no sequencing key and is the path-precise error: array_struct_sequencing_pointer_same.c). Every tag is to confirm at the joined run
// Pre-registered from record 0064 items 5 and 11 (Part B), before any run: rejected (tags by array_reject_address_null_test.c and array_reject_a_i_eq_a_i_postinc.c)
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

int truth(void)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  if (&v[1]) {  // <- here
    return v[0].x;
  }
  return 0;
}

int member_null(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  return (&v[i].x == NULL) ? 0 : v[0].x;  // <- here
}

int negation(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  return !&v[i] ? 0 : v[0].x;  // <- here
}

int pointer_null(struct P *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return (&p[i] != NULL) ? p->x : 0;  // <- here
}

int in_check(void)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  CERASTES_CHECK(&v[1] != NULL);  // <- here
  return v[0].x;
}

int same_const(void)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  v[0].x = v[0].x++;  // <- here
  return v[0].y;
}

int same_var(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  v[i].x = v[i].x++;  // <- here
  return v[0].y;
}
