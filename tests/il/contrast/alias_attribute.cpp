// expect: golden
// Contrast probe: an attribute on an alias declaration belongs to the alias.
// `al` is written on the alias FT2 (a function type); `g` and `h` are
// functions declared with that type and must not report it, so `al` appears
// exactly once, as an entity-level `on typedef FT2` line and not under `g`
// or `h`.  `real` has an ordinary attribute after its parameter list, which
// EDG puts on the function's type: it must still be reported `on type`, so
// that the fix for aliases does not drop attributes that really are on a
// routine's type.

using FT2 [[cerastes::al(1)]] = int(int);

FT2 g;
FT2 h;

int real(int x) [[cerastes::post(x > 0)]];
