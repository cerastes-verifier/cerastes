// expect: golden
// Annotation-free copy of tests/cxx/templates/generic_body.cpp from
// verifast branch generic2, commit 0fb63506a51cae559ae92348eb54ba7f7be0bf6d.
// Whole-line //@ annotations are replaced by empty lines and trailing
// //~ should_fail markers are removed; no other change.
// Line offset: line N here is line N-6 in the original (header is 6 lines).
// Constructs that function templates can use in their body, which is verified
// once with abstract type parameters.

template <typename T>
T copy_through_locals(T x)


{
  T y = x;
  T z;
  z = y;


  return z;
}

template <typename T, typename U>
U second(T x, U y)


{
  // Calls that do not depend on a type parameter are resolved in the template.
  int i = copy_through_locals(1);

  return y;
}

template <typename T>
T declared_first(T x);



template <typename T>
T declared_first(T x)


{
  return x;
}

template <typename T>
void assign(T &r, const T v)


{
  r = v;
}

template <typename T>
T &same_ref(T &r)


{
  return r;
}

int main()


{
  long l = copy_through_locals(5L);

  char c = second(1, 'x');

  unsigned u = declared_first(4u);

  int i = 0;
  assign(i, 9);

  int &r = same_ref(i);
  return 0;
}
