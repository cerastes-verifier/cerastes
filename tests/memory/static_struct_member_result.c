// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 7 (retires static_struct_member_result_false_alarm.c, the same program plus a read back): ensures(r, owns(*r)) of a function returning &s.x of a static struct is covered by the implicit owns(s), per leaf of the atom's pointee type (cov := r == &s.x || r == &s.y); the caller reads and writes *p through its own implicit atom on s
// Pre-registered from record 0054 item 7 (S39 (e), s39_e_static_struct_cover.vpr): verifies in both modes
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int *where(void)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  static struct S s;
  s.x = 1;
  return &s.x;
}

int main(void)
{
  int *p = where();
  *p = 2;
  CERASTES_CHECK(*p == 2);
  return 0;
}
