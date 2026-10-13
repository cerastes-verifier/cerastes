// expect: golden
// Contrast probe: a callee's default argument is not part of the caller's
// body.  `h` is defined with the default argument `1 << 2`, and called twice
// as `h()` from the function template `tmpl` and once from the plain function
// `plain`.  A call that uses a default argument makes EDG attach that
// argument's expression to the call; the walker must not follow it into the
// callers.  The prototype of `tmpl` must show exactly one `eok_shiftl`, the
// genuine `1 << 3` written in its body, at its own position (line 19); the
// default argument's shift (line 13) must not appear under `tmpl` or under
// `plain`.  Where it appears at all (under `h`, as part of its declaration) is
// recorded in the golden, after checking it against il_display.

int h(int a = 1 << 2) { return a; }

int plain() { return h(); }

template <typename T>
T tmpl(T v) {
  int g = 1 << 3;
  return v + h() + h() + g;
}
