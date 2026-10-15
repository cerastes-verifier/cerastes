// expect: golden
// expect: native-ok
// twin-exempt: regression test for a false positive: the user's own forall in a template is not cerastes::forall
// Pins: a template whose body calls a user's algo::forall(r, p) is ordinary code: no spec_call_in_code (the name is looked up in the user's namespace, not in cerastes).
namespace algo {
template <class R, class P>
bool forall(const R& r, P p)
{
  for (auto& e : r)
    if (!p(e))
      return false;
  return true;
}
}

template <class R, class P>
bool all_of(const R& r, P p)
{
  return algo::forall(r, p);
}

int main()
{
  int a[3] = {1, 2, 3};
  return all_of(a, [](int v) { return v > 0; }) ? 0 : 1;
}
