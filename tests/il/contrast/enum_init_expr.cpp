// expect: golden
// Contrast probe: an enumerator initializer is not part of any function body.
// `enum E { EA = 1 << 4 };` has an operation (`1 << 4`, eok_shiftl) in the
// initializer of EA.  The enumerator is used twice in the body of the function
// template `tmpl` and once in the plain function `plain`.  No `op eok_shiftl`
// line may appear under any routine, and every `op` line must belong to the
// routine's own body: `tmpl` has `op` lines for its `+` operations only.
// If the walker followed the enumerator's initializer from the uses, the
// shift would show up under `tmpl`, and the G1 operation checks would be
// attributing operations to the wrong routine.

enum E { EA = 1 << 4 };

int plain(int x) { return x + EA; }

template <typename T>
T tmpl(T v) {
  T a = v + EA;
  return a + EA;
}
