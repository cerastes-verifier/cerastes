// expect: verified
// expect: native-ok
// mode: c23, c++20
// twin-exempt: an inert declaration, nothing in it can fail; its must-fail form is the rejected pin reject_not_invalidating_typedef_used.c (as init_uncalled.c's reject_marker_uncalled_*)
// Pins: 0042 item 4 (record 0058 follows it, revised 2026-11-10): not_invalidating in the parameter list of a function-pointer typedef that nothing uses reaches no vir::Param and is inert, as must_init and ref_to_uninit are (observed today with must_init and ref_to_uninit: build/m4.5d2b2d/emul2/t_unused.c, t_unused_ref.c)
// Pre-registered from record 0042 item 4: verifies in both modes
#include <cerastes.h>

typedef void (*callback_t)(int *p CERASTES_NOT_INVALIDATING);

int main(void)
{
  return 0;
}
