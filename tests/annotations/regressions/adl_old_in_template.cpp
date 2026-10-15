// expect: golden
// expect: native-ok
// twin-exempt: regression test for a false positive: an unqualified old(x) found by argument-dependent lookup on a user type is not cerastes::old
// Pins: a template calling the unqualified old(x) on a user type: the call resolves to the user's old in the user's namespace, so no spec_call_in_code and no old_outside_ensures.
namespace hist {
struct V {
  int v;
};
inline V old(V x)
{
  return x;
}
}

template <class T>
T prev(T x)
{
  return old(x);
}

int main()
{
  hist::V a{1};
  return prev(a).v;
}
