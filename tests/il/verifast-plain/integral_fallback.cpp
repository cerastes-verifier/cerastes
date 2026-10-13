// expect: golden
// Annotation-free copy of tests/cxx/templates/integral_fallback.cpp from
// verifast branch generic2, commit 0fb63506a51cae559ae92348eb54ba7f7be0bf6d.
// Whole-line //@ annotations are replaced by empty lines and trailing
// //~ should_fail markers are removed; no other change.
// Line offset: line N here is line N-6 in the original (header is 6 lines).
// A function template constrained by std::integral whose generic function
// fails is verified per specialization instead, if something instantiates it.

#include <concepts>

// Does not verify generically: inc(150) does not fit in a signed char. But
// nothing instantiates it for signed char.
template <std::integral T>
T inc(const T v)


{
  return (T)(v + 1);
}

// Does not type-check generically: shifts of values of an integral type
// parameter are not supported, even in annotations.
template <std::integral T>
T twice(const T v)


{
  return (T)(v + v);
}

int main()


{
  int i = inc(150);

  long l = inc(199L);

  int t = twice(6);

  return 0;
}
