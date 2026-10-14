// A "ghost" field written as an ordinary attributed field still takes storage.
struct S {
  int x;
  [[cerastes::ghost]] int y;
};
static_assert(sizeof(S) == sizeof(int), "a spec-only field must not change the layout");
