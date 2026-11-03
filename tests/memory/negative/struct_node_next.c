// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/struct_node.c
// Pins: 0043 item 5: n->next is m, which the function only knows live: reading its v is not covered by any atom
#include <cerastes.h>
#include <stddef.h>

struct node {
  int v;
  struct node *next;
};

int second(struct node *n, struct node *m)
    CERASTES_REQUIRES(cerastes_owns(*n) && cerastes_live(m))
{
  n->next = m;
  return n->next->v;  // <- here
}
