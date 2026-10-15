// expect: frontend-error unrecognized_attribute @ L6
// A name that cerastes does not have (this one is never available, on purpose).
// tag::bad[]
int f(int x)
{
  [[cerastes::assert(x > 0)]]; // <- error
  return x;
}
// end::bad[]
