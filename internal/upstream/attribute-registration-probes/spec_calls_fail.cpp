// Must-fail twins for spec_calls.cpp: EDG itself diagnoses an undeclared name
// and a bad call, at the attribute's own line.
namespace cerastes {
template <class T> bool owns(const T&);
template <class T> T old(const T&);
}
struct counter { int val; };
void bad1(counter* c) __attribute__((enable_if(cerastes::owns_all(c->val), "undeclared")));
void bad2(counter* c) __attribute__((enable_if(cerastes::owns(c->val, 1), "wrong arity")));
