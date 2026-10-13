// expect: golden
// Contrast probe: instances of one function template with two different
// unnamed types must be told apart in their `template ... args` lines.
// `pick` is instantiated with an unnamed struct type and with an unnamed enum
// type (both reached through decltype of a variable).  The two instances must
// print distinct arguments, `T=struct <unnamed>` and `T=enum <unnamed>`, with
// the type names exactly as il_display gives them (minus the address).  If
// both printed the same text, or a placeholder, the instances could not be
// told apart in a golden.

struct { int a; } anon_s;
enum { AE1, AE2 } anon_e;

template <typename T>
T pick(T v) { return v; }

int use() {
  pick(anon_s);
  pick(anon_e);
  return 0;
}
