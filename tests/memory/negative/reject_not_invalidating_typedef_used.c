// expect: rejected cerastes-unsupported_type @ L10
// mode: c23, c++20
// twin: tests/memory/not_invalidating_typedef_unused.c
// Pins: 0042 item 4 (record 0058 follows it): the same typedef used as a parameter type: the pointer to a function is unsupported_type at the use (M4.7), whether or not the typedef carries a marker (observed today with must_init, ref_to_uninit and no marker: build/m4.5d2b2d/emul2/t_used.c, t_used_ref.c, t_unmarked_used.c)
// Pre-registered from record 0042 item 4: rejected unsupported_type at the function that uses the typedef
#include <cerastes.h>

typedef void (*callback_t)(int *p CERASTES_NOT_INVALIDATING);

void run(callback_t cb) { (void)cb; }  // <- here

int main(void)
{
  return 0;
}
