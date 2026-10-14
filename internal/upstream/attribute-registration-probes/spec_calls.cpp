// The owner's bump() example, with Clang's enable_if standing in for the
// attributes cerastes will register. Spec functions are ordinary declarations.
namespace cerastes {
template <class T> bool owns(const T&);
template <class T> T old(const T&);
}
struct counter { int val; };
#define INT_MAX 2147483647
void bump(counter* c)
  __attribute__((enable_if(cerastes::owns(c->val) && c->val < INT_MAX, "pre")))
  __attribute__((enable_if(cerastes::owns(c->val) && c->val == cerastes::old(c->val) + 1, "post")));
void bump(counter* c) { c->val += 1; }
