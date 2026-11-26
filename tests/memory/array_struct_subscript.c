// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 5 (Part B): p[i].x on a pointer to a structure with owns(*p) and i == 0: a read (get is the program of the retired pin array_reject_struct_subscript.c), a write, an element read into a structure value (t = p[i]) and an element written from one (p[i] = t); set, read_elem and write_elem are the programs of the staging pin array_reject_struct_elem_ptr_forms.c
// Pre-registered from record 0064 item 5 (Part B), before any run: verifies
#include <cerastes.h>

struct S {
  int x;
};

struct P {
  int x;
  int y;
};

int get(struct S *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return p[i].x;
}

void set(struct P *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p[i].x = 1;
}

int read_elem(struct P *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  struct P t;
  t = p[i];
  return t.x;
}

void write_elem(struct P *p, int i, struct P t)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p[i] = t;
}
