// expect: golden
// Contrast probe: ordinary attributes must not be reported as unreg_ns::
// attributes.  The file has [[nodiscard]], [[maybe_unused]], [[likely]] and
// [[gnu::always_inline]], and one [[unreg_ns::x(n > 0)]] on `mixed`.  Only that
// one may be reported with namespace unreg_ns; if the walker reported a
// `unreg_ns::` attribute on any other entity, the G1 check 5 goldens would
// pass vacuously.
// What the walker does with ordinary attributes (found by running it, recorded
// in the golden): it does not skip them.  [[nodiscard]] and [[gnu::...]] are
// reported on their routine, [[maybe_unused]] on its parameter, and [[likely]]
// on the compound statement after the `if` condition, which is the statement
// it appertains to.  [[maybe_unused]] on the local variable is reported `on
// variable` (an earlier walker skipped it).  Warning #2803 for the unreg_ns
// namespace is expected on stderr; none appears for the others.

[[nodiscard]] int checked(int n);

[[gnu::always_inline]] inline int fast(int n) { return n + 1; }

int mixed([[maybe_unused]] int n, int m) [[unreg_ns::x(n > 0)]];

int branches(int n) {
  if (n > 0) [[likely]] {
    return 1;
  }
  [[maybe_unused]] int unused_local = n;
  return 0;
}
