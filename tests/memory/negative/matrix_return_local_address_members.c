// expect: rejected cerastes-return_local_address @ L28
// expect: rejected cerastes-return_local_address @ L34
// expect: rejected cerastes-return_local_address @ L40
// expect: rejected cerastes-return_local_address @ L46
// expect: frontend-warning returning_ptr_to_local_variable @ L28
// expect: frontend-warning returning_ptr_to_local_variable @ L34
// expect: frontend-warning returning_ptr_to_local_variable @ L40
// expect: frontend-warning returning_ptr_to_local_variable @ L46
// mode: c23, c++20
// twin: tests/memory/matrix_return_local_ok.c
// Pins: 0064 item 5 and 0039 item 6 (audit round E1, finding 1): the forms of matrix_return_local_address.c over a matrix MEMBER of a local structure (u.m[1], &u.m[1][2]), the array member of an element of a local array of structures (v[1].a, &v[1].a[1]) (the depth 3 local's two rows, c[1][1] and &c[1][1][0], are held outside the tree until slice 4 builds them: question 370). They run once slice 3 has landed: until then the encoder rejects these locals for another reason, so this file stays out of the joined snapshots before it. One function per form
// Pre-registered from record 0064 item 5 (Part E), before any run: return_local_address at each return; corrected once after the first run on j4a (question 160): the front end's returning_ptr_to_local_variable warning at each of the four returns is registered (as in matrix_return_local_address.c), the header grew by four lines so every line moved by +4
#include <cerastes.h>

struct U {
  int m[2][3];
  int n;
};

struct E {
  int a[2];
  int n;
};

int *member_row(void)
{
  struct U u = {{{1, 2, 3}, {4, 5, 6}}, 7};
  return u.m[1];  // <- here
}

int *member_cell(void)
{
  struct U u = {{{1, 2, 3}, {4, 5, 6}}, 7};
  return &u.m[1][2];  // <- here
}

int *elem_array(void)
{
  struct E v[2] = {{{1, 2}, 3}, {{4, 5}, 6}};
  return v[1].a;  // <- here
}

int *elem_cell(void)
{
  struct E v[2] = {{{1, 2}, 3}, {{4, 5}, 6}};
  return &v[1].a[1];  // <- here
}
