// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 (retires reject_ref_to_uninit_static_local.c: ref_to_uninit on a static local pointer): default mode, the marker documents intent and changes no Viper: the function stores malloc's block in the marked static local, writes it, frees it and resets it (a read of the unknown entry value of a non-owning static pointer is lifetime today, so the body does not test it first)
// Pre-registered from record 0054 item 5 ("Marked globals and static locals"): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int f(void)
    CERASTES_ENSURES(r, r == 0 || r == 1)
{
  CERASTES_REF_TO_UNINIT static int *s = NULL;
  s = (int *)malloc(sizeof(int));
  if (s == NULL) {
    return 1;
  }
  *s = 3;
  free(s);
  s = NULL;
  return 0;
}

int main(void)
{
  return f();
}
