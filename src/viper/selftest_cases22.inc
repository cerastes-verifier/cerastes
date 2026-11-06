// (included by selftest.cpp) Owning locals in loops (decision 0052 item 3, Loops; M4.5d2b1): an owning local declared
// outside a loop that the loop uses or assigns is carried by the
// loop's invariant as its whole block at its CURRENT value (`held_v == (v != null)`, the cells, the liveness cell, the
// token, the facts of a block), not as an entry-keyed heap
// place (0047 item 10); an owner the loop merely beside-calls keeps only `held_v == (v != null)`.  The entry and
// back-edge ENDING ROWS `v != null ==> held_v` (`permission`:
// "set it to NULL after freeing it or handing it on") precede the invariant, so a violation is a user error at the
// loop, never Silicon's own check; the back-edge leak row exempts
// the owner's token; a vlibc heap program also snapshots the allocator (`tve`) the loop must keep.
namespace {

void define_owning_loops()
{
    auto PT = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto notnull = [](Kit& k, const Var* p)
    {
        return k.ne(k.pn(p), k.nul(k.ptr(k.i32)));
    };
    auto owning_local = [](Kit& k, Function* f, const char* name)
    {
        Var* v {k.local(f, name, k.ptr(k.i32))};
        v->owning = true;
        return v;
    };
    constexpr Allocator V {Allocator::Vlibc};
    constexpr Allocator P {Allocator::Platform};

    // void f() { owning int *v = [malloc() | NULL]; int i = 0; while (i < 3) { BODY; i++; } [free(v);] }
    // mode 0: BODY = free(v); v = malloc();                (the block of the previous iteration is freed first:
    // verifies)
    // mode 1: BODY = v = malloc();                         (the reassignment row: leak at the assignment)
    // mode 2: BODY = free(v);                              (v not reset: the back-edge row, permission at the body's
    // end; the loop's exit leaves a block held at the return)
    // mode 3: BODY = if (v != NULL) { *v = 1; }            (v used, not assigned: its cells are in the invariant)
    // mode 4: BODY = if (v != NULL) { *v = 1; free(v); }   (the same, then freed and not reset: the back-edge row)
    // mode 5: BODY = g();                                  (a call that does not name v: only `held_v == (v != null)`
    // is carried)
    // `null_entry`: v starts as NULL; `free_after`: free(v) follows the loop; the SrcLoc is the failing node's (the
    // assignment, or the body's end).
    struct Built
    {
        SrcLoc at;
        SrcLoc fn_end;
    };
    const auto simple = [=](Kit& k, int mode, bool null_entry, bool free_after) -> Built
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* g {k.fn("g", k.vd)};
        g->body = k.blkc({});
        k.done(g);
        Function* f {k.fn("f", k.vd)};
        Var* v {owning_local(k, f, "v")};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body;
        SrcLoc where;
        if (mode == 0)
        {
            body.push_back(k.es(k.call(fr, {k.pn(v)})));
        }
        if (mode == 0 || mode == 1)
        {
            const Stmt* a {k.asg(v, k.call(m, {}))};
            where = a->loc;
            body.push_back(a);
        }
        if (mode == 2)
        {
            body.push_back(k.es(k.call(fr, {k.pn(v)})));
        }
        if (mode == 3 || mode == 4)
        {
            std::vector<const Stmt*> in {k.store(k.pn(v), N(k, "1"))};
            if (mode == 4)
            {
                in.push_back(k.es(k.call(fr, {k.pn(v)})));
            }
            body.push_back(k.iff(notnull(k, v), k.blk(in)));
        }
        if (mode == 5)
        {
            body.push_back(k.es(k.call(g, {})));
        }
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* bs {k.blkc(body)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        std::vector<const Stmt*> top {k.decl(v, null_entry ? k.nul(PT(k)) : k.call(m, {})), k.decl(i, N(k, "0")), lp};
        if (free_after)
        {
            top.push_back(k.es(k.call(fr, {k.pn(v)})));
        }
        const Stmt* fb {k.blkc(top)};
        f->body = fb;
        k.done(f);
        return Built {mode >= 2 && mode != 5 && mode != 3 ? bs->end_loc : where, fb->end_loc};
    };
    lc("olp_replace",
       "owning v freed and reallocated in the loop, freed after it: the loop carries v's whole block at its current value, the reassignment row finds the old block freed, the back-edge leak row exempts v's token",
       [=](Kit& k)
       {
           simple(k, 0, false, true);
       });
    lc("olp_replace_null_entry",
       "the same from an owner that is NULL at the entry: free(NULL) the first time, `held_v == (v != null)` carried",
       [=](Kit& k)
       {
           simple(k, 0, true, true);
       });
    lc("olp_replace_leak",
       "the loop allocates into owning v without freeing the block it held: the reassignment row fails at the assignment (leak), once",
       [=](Kit& k)
       {
           k.fail("leak", simple(k, 1, false, true).at);
       });
    lc("olp_free_not_reset",
       "the loop frees owning v and leaves it pointing at the freed block: the back-edge row `v != null ==> held_v` fails at the end of the body (permission); the exit of the loop, where the invariant says v still holds its block, is a leak at the end of the function",
       [=](Kit& k)
       {
           const Built b {simple(k, 2, false, false)};
           // the held row fails and ends the path; the rows of the variable's place behind it are the same loss
           k.fail_only("permission", b.at, 7U, 0U);
           k.fail("leak", b.fn_end);
       });
    lc("olp_used",
       "owning v used (`*v = 1`) in the loop but not assigned: v's cells are in the invariant, so the store is framed",
       [=](Kit& k)
       {
           simple(k, 3, false, true);
       });
    lc("olp_used_freed",
       "the loop stores through v and frees it without resetting it: the back-edge row fails (permission) at the end of the body",
       [=](Kit& k)
       {
           const Built b {simple(k, 4, false, false)};
           k.fail_only("permission", b.at, 7U, 0U);
           k.fail("leak", b.fn_end);
       });
    lc("olp_beside_call",
       "a loop that calls a function and does not name owning v carries only `held_v == (v != null)`: v is NULL here, so nothing is held at the end",
       [=](Kit& k)
       {
           simple(k, 5, true, false);
       });
    lc("olp_beside_call_leak",
       "the same with an allocated v that is never freed: leak at the end of the function",
       [=](Kit& k)
       {
           k.fail("leak", simple(k, 5, false, false).fn_end);
       });

    // void f() { owning int *p = malloc(); owning int *q = malloc(); int i = 0; while (i < 3) { [owning] int *t = p; p
    // = q; q = t; i++; } free(p); free(q); }
    const auto swap = [=](Kit& k, bool owning_temp) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd)};
        Var* p {owning_local(k, f, "p")};
        Var* q {owning_local(k, f, "q")};
        Var* i {k.local(f, "i", k.i32)};
        Var* t {owning_temp ? owning_local(k, f, "t") : k.local(f, "t", PT(k))};
        const Stmt* pq {k.asg(p, k.pn(q))};
        const Stmt* bs {k.blkc({k.decl(t, k.pn(p)), pq, k.asg(q, k.pn(t)), k.asg(i, k.add(k.v(i), N(k, "1")))})};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        f->body = k.blkc({k.decl(p, k.call(m, {})), k.decl(q, k.call(m, {})), k.decl(i, N(k, "0")), lp,
                          k.es(k.call(fr, {k.pn(p)})), k.es(k.call(fr, {k.pn(q)}))});
        k.done(f);
        return pq->loc;
    };
    lc("olp_swap",
       "two owners swapped through an owning temporary declared in the body: each reassignment finds the old block held by an owner that remains",
       [=](Kit& k)
       {
           swap(k, true);
       });
    lc("olp_swap_plain_temp",
       "the same through a plain temporary: when p is overwritten its block is held only by t, which is not an owner (leak at p = q)",
       [=](Kit& k)
       {
           k.fail("leak", swap(k, false));
       });

    // void f() { owning int *v = malloc(); int i = 0; while (i < 3) { free(v); v = malloc(); [while (j < 2) { g(); j++
    // }] i++; } free(v); }
    // the inner loop names no owner and calls g: the owner is carried by both loops
    const auto nested = [=](Kit& k, bool leak) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* g {k.fn("g", k.vd)};
        g->body = k.blkc({});
        k.done(g);
        Function* f {k.fn("f", k.vd)};
        Var* v {owning_local(k, f, "v")};
        Var* i {k.local(f, "i", k.i32)};
        Var* j {k.local(f, "j", k.i32)};
        const Stmt* inner {k.whl(k.lt(k.v(j), N(k, "2")), {k.cl(k.range(k.v(j), k.i32, "0", "2"))},
                                 {k.cl(k.sub(N(k, "2"), k.v(j)))},
                                 k.blkc({k.es(k.call(g, {})), k.asg(j, k.add(k.v(j), N(k, "1")))}))};
        std::vector<const Stmt*> body;
        if (!leak)
        {
            body.push_back(k.es(k.call(fr, {k.pn(v)})));
        }
        const Stmt* again {k.asg(v, k.call(m, {}))};
        body.push_back(again);
        body.push_back(k.decl(j, N(k, "0")));
        body.push_back(inner);
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* outer {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                 {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(body))};
        f->body = k.blkc({k.decl(v, k.call(m, {})), k.decl(i, N(k, "0")), outer, k.es(k.call(fr, {k.pn(v)}))});
        k.done(f);
        return again->loc;
    };
    lc("olp_nested",
       "an owner carried by an outer loop that replaces it and an inner loop that only calls: both loops carry it",
       [=](Kit& k)
       {
           nested(k, false);
       });
    lc("olp_nested_leak", "the same without the free before the reallocation: leak at the assignment in the outer loop",
       [=](Kit& k)
       {
           k.fail("leak", nested(k, true));
       });

    // void f() { owning int *v = malloc(); if (v == NULL) { return; } *v = 0; int *a = v; int i = 0; while (i < 3) {
    // BODY; i++; } free(v); }
    // A non-owning alias of an owner's block is a heap place of the loop beside the owner's invariant: its guards say
    // `a != v` (the owner's cells are the alias's when they are one).
    // mode 0: BODY = *a = i;                        (v is not named: not carried at all)
    // mode 1: BODY = *a = i; g();                   (v is carried for its token only)
    // mode 2: BODY = *a = i; *v = i;                (v is used: its whole block is in the invariant beside the place of
    // a)
    // mode 3: BODY = a = v; *a = i;                 (the alias is assigned)
    // mode 4: BODY = *a = i; free(v); v = malloc(); if (v == NULL) { return; }   (the alias dangles: the back edge says
    // so, a user error, not an internal one)
    const auto alias = [=](Kit& k, int mode) -> Built
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* g {k.fn("g", k.vd)};
        g->body = k.blkc({});
        k.done(g);
        Function* f {k.fn("f", k.vd)};
        Var* v {owning_local(k, f, "v")};
        Var* a {k.local(f, "a", PT(k))};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body;
        if (mode == 3)
        {
            body.push_back(k.asg(a, k.pn(v)));
        }
        const Stmt* alias_store {k.store(k.pn(a), k.v(i))};
        body.push_back(alias_store);
        if (mode == 1)
        {
            body.push_back(k.es(k.call(g, {})));
        }
        if (mode == 2)
        {
            body.push_back(k.store(k.pn(v), k.v(i)));
        }
        if (mode == 4)
        {
            body.push_back(k.es(k.call(fr, {k.pn(v)})));
            body.push_back(k.asg(v, k.call(m, {})));
            body.push_back(k.iff(k.eq(k.pn(v), k.nul(PT(k))), k.blk({k.ret(nullptr)})));
        }
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* bs {k.blkc(body)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        const Stmt* fb {k.blkc({k.decl(v, k.call(m, {})), k.iff(k.eq(k.pn(v), k.nul(PT(k))), k.blk({k.ret(nullptr)})),
                                k.store(k.pn(v), N(k, "0")), k.decl(a, k.pn(v)), k.decl(i, N(k, "0")), lp,
                                k.es(k.call(fr, {k.pn(v)}))})};
        f->body = fb;
        k.done(f);
        return Built {mode == 4 ? alias_store->loc : bs->end_loc, f->end_loc};
    };
    lc("olp_alias", "a non-owning alias of an owner's block used in a loop that does not name the owner",
       [=](Kit& k)
       {
           alias(k, 0);
       });
    lc("olp_alias_call", "the same with a call in the loop: the owner is carried for its token, the alias is a place",
       [=](Kit& k)
       {
           alias(k, 1);
       });
    lc("olp_alias_used", "the alias and the owner both used: the owner's cells are in the invariant beside the alias's place, not twice",
       [=](Kit& k)
       {
           alias(k, 2);
       });
    lc("olp_alias_assigned", "the alias assigned from the owner in the loop",
       [=](Kit& k)
       {
           alias(k, 3);
       });
    lc("olp_alias_freed",
       "the owner's block is freed and replaced while the alias is kept: the alias dangles at the next iteration (its store fails: permission)",
       [=](Kit& k)
       {
           k.fail("permission", alias(k, 4).at);
       });

    // void f() { owning int *v = malloc(); if (v == NULL) { return; } *v = 0; int i = 0; INV while (i < 3) { *v = i;
    // i++; } free(v); }
    // mode 0: INV = invariant(*v >= 0)                  (the user's invariant reads the owner's cell: framed by the
    // loop's own clauses, which come first)
    // mode 1: INV = invariant(*v >= 1)                  (false at the entry: invariant-entry at the clause)
    // mode 2: INV = invariant(owns(*v))                 (a written atom on an owner: the loop already carries the
    // block; an error at the atom, never an internal invariant)
    const auto user_inv = [=](Kit& k, int mode) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd)};
        Var* v {owning_local(k, f, "v")};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* bs {k.blkc({k.store(k.pn(v), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))})};
        SrcLoc where;
        std::vector<Clause> inv {k.cl(k.range(k.v(i), k.i32, "0", "3"))};
        if (mode == 2)
        {
            const Expr* at {k.b.atom(AtomKind::Owns, k.pn(v), -1, k.at())};
            where = at->loc;
            inv.push_back(k.cl(at));
        }
        else
        {
            Clause c {k.cl(k.ge(k.deref(k.pn(v)), N(k, mode == 0 ? "0" : "1")))};
            where = c.loc;
            inv.push_back(c);
        }
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), inv, {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        f->body = k.blkc({k.decl(v, k.call(m, {})), k.iff(k.eq(k.pn(v), k.nul(PT(k))), k.blk({k.ret(nullptr)})),
                          k.store(k.pn(v), N(k, "0")), k.decl(i, N(k, "0")), lp, k.es(k.call(fr, {k.pn(v)}))});
        k.done(f);
        return where;
    };
    lc("olp_user_invariant_reads_owner", "a user invariant that reads `*v` of an owning loop variable is framed by the loop's own clauses",
       [=](Kit& k)
       {
           user_inv(k, 0);
       });
    lc("olp_user_invariant_entry", "the same invariant false at the entry (invariant-entry at the clause)",
       [=](Kit& k)
       {
           k.fail("invariant-entry", user_inv(k, 1));
       });
    lc("olp_user_atom_on_owner", "a written atom on an owning loop variable: an error at the atom (permission), the block is carried by the loop already",
       [=](Kit& k)
       {
           k.fail("permission", user_inv(k, 2));
       });

    // vlibc: void f() { owning int *q = vlibc_malloc(); if (q == NULL) { return; } int i = 0; while (i < 3) {
    // vlibc_free(q); q = [vlibc_malloc() | malloc()]; i++; } vlibc_free(q); }
    // the loop keeps the allocator of the block it held at its entry (`tve`): a block of the other allocator at the
    // back edge is a row of its own (permission at the end of the body)
    // mode 0: BODY = vlibc_free(q); q = vlibc_malloc();     verifies (every assignment is vlibc's: the invariant says
    // so)
    // mode 1: BODY = q = vlibc_malloc();                    leak at the assignment
    // mode 2: BODY = vlibc_free(q); q = malloc();           q holds vlibc's block at the entry and the loop gives it
    // the platform's: the allocator row fails at the loop's entry
    // mode 3: as mode 0, with no NULL test: the owner may be NULL at the entry, which the constant class carries (the
    // entry snapshot could not)
    // mode 4: BODY = *q = i;                                (q used, not assigned: the allocator of the entry, `tve`)
    // mode 5: BODY = vlibc_free(q);                         (q not reset: the back-edge row, permission at the end of
    // the body)
    const auto vloop = [=](Kit& k, int mode) -> Built
    {
        Function* vm {heap_alloc_view(k, "vlibc_malloc", k.i32, false, V)};
        Function* pm {heap_alloc_view(k, "malloc", k.i32, false, P)};
        Function* vf {heap_free_view(k, k.i32, V)};
        Function* f {k.fn("f", k.vd)};
        Var* q {owning_local(k, f, "q")};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body;
        SrcLoc where;
        if (mode == 0 || mode == 2 || mode == 3 || mode == 5)
        {
            body.push_back(k.es(k.call(vf, {k.pn(q)})));
        }
        if (mode <= 3)
        {
            const Stmt* a {k.asg(q, k.call(mode == 2 ? pm : vm, {}))};
            where = a->loc;
            body.push_back(a);
        }
        if (mode == 4)
        {
            body.push_back(k.store(k.pn(q), k.v(i)));
        }
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* bs {k.blkc(body)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        std::vector<const Stmt*> top {k.decl(q, k.call(vm, {}))};
        if (mode != 3)
        {
            // a NULL entry cannot carry an allocator it does not know (the entry class is the platform's then)
            top.push_back(k.iff(k.eq(k.pn(q), k.nul(PT(k))), k.blk({k.ret(nullptr)})));
        }
        if (mode == 4)
        {
            top.push_back(k.store(k.pn(q), N(k, "0")));
        }
        top.push_back(k.decl(i, N(k, "0")));
        top.push_back(lp);
        if (mode != 5)
        {
            top.push_back(k.es(k.call(vf, {k.pn(q)})));
        }
        const Stmt* fb {k.blkc(top)};
        f->body = fb;
        k.done(f);
        return Built {mode == 2 ? lp->loc : (mode == 5 ? bs->end_loc : where), fb->end_loc};
    };
    lc("olp_vlibc_replace",
       "an owner holding a vlibc block, freed with vlibc_free and reallocated by vlibc_malloc in the loop: every assignment is vlibc's, so the invariant says so",
       [=](Kit& k)
       {
           vloop(k, 0);
       });
    lc("olp_vlibc_replace_leak", "the same without the vlibc_free: leak at the assignment",
       [=](Kit& k)
       {
           k.fail("leak", vloop(k, 1).at);
       });
    lc("olp_vlibc_other_allocator",
       "the owner holds vlibc's block at the entry and the loop gives it the platform's: the allocator row fails at the loop's entry (permission), never Silicon's invariant check",
       [=](Kit& k)
       {
           // the held and whole-block rows hold; the allocator row fails
           k.fail_only("permission", vloop(k, 2).at, 3U, 2U);
       });
    lc("olp_vlibc_null_entry",
       "the same loop as the first, from an owner that may be NULL at the entry (no test): the allocator is the assignments', not the entry's",
       [=](Kit& k)
       {
           vloop(k, 3);
       });
    lc("olp_vlibc_used", "an owner used but not assigned by a loop of a vlibc heap program: the allocator of its block at the entry is carried (tve)",
       [=](Kit& k)
       {
           vloop(k, 4);
       });
    lc("olp_vlibc_used_freed", "the loop frees it without resetting it: the back-edge row fails (permission) at the end of the body",
       [=](Kit& k)
       {
           const Built b {vloop(k, 5)};
           k.fail_only("permission", b.at, 7U, 0U);
           k.fail("leak", b.fn_end);
       });
}

}

namespace {

void define_owning_loops_audit2()
{
    struct Built
    {
        SrcLoc at;
        SrcLoc fn_end;
    };
    auto PT = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // ======== audit round 2 (M4.5d2b1): partners of block sites, given liveness, pure rows, owners assigned in
    // expressions, parked formals ========
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto block = [](Kit& k, const Expr* ptr)
    {
        return k.b.atom(AtomKind::OwnsBlock, ptr, -1, k.at());
    };
    auto owns = [](Kit& k, const Expr* ptr, int amount)
    {
        return k.b.atom(AtomKind::Owns, ptr, amount, k.at());
    };
    auto live = [](Kit& k, const Expr* ptr, int amount)
    {
        return k.b.atom(AtomKind::Live, ptr, amount, k.at());
    };

    // B2: void lend(int *p, int c) requires(c > 5 ? owns_block(p) : true) ensures(c > 0 ? owns_block(p) : true);   (no
    // body: trusted)
    // the `ensures` block site is returned on paths (0 < c <= 5) where the `requires` took nothing: f__wf's partner
    // row, a permission error at the ensures atom
    // [heap guards: void lend(int *p, int *q) requires(owns(*q) && (*q > 5 ? owns_block(p) : true)) ensures(owns(*q) &&
    // *q == old(*q) && (*q > 0 ? owns_block(p) : true));]
    const auto lend = [=](Kit& k, bool heap_guard) -> SrcLoc
    {
        Function* f {k.fn("lend", k.vd, {{"p", PT(k)}, {heap_guard ? "q" : "c", heap_guard ? PT(k) : k.i32}})};
        k.trusted(f, "audit 2 B2: a borrow whose ensures guard is weaker than its requires guard");
        const Expr* cp {heap_guard ? k.deref(FP(k, f, 1)) : FP(k, f, 1)};
        const Expr* cp2 {heap_guard ? k.deref(FP(k, f, 1)) : FP(k, f, 1)};
        const Expr* a {block(k, FP(k, f, 0))};
        const Expr* en {block(k, FP(k, f, 0))};
        if (heap_guard)
        {
            k.req(f, k.land(owns(k, FP(k, f, 1), 0), k.cond(k.gt(cp, N(k, "5")), a, k.yes())));
            k.ens(f, k.land(k.land(owns(k, FP(k, f, 1), 0), k.eq(k.deref(FP(k, f, 1)), k.old(k.deref(FP(k, f, 1))))),
                            k.cond(k.gt(cp2, N(k, "0")), en, k.yes())));
        }
        else
        {
            k.req(f, k.cond(k.gt(cp, N(k, "5")), a, k.yes()));
            k.ens(f, k.cond(k.gt(cp2, N(k, "0")), en, k.yes()));
        }
        k.done(f);
        return en->loc;
    };
    lc("pb_block_path", "a bodiless borrow that returns its block on a path where its requires took none: the f__wf partner row fails at the ensures atom (permission)",
       [=](Kit& k)
       {
           k.fail("permission", lend(k, false));
       });
    lc("pb_block_path_heap",
       "the same with guards that read the heap (*q): the partner row is an obligation at the atom (permission)",
       [=](Kit& k)
       {
           k.fail("permission", lend(k, true));
       });
    lc("pb_block_path_same_guard", "a borrow whose requires and ensures guards are one and the same: no row",
       [=](Kit& k)
       {
           Function* f {k.fn("lend", k.vd, {{"p", PT(k)}, {"c", k.i32}})};
           k.trusted(f, "audit 2 B2: the same guard");
           k.req(f, k.cond(k.gt(FP(k, f, 1), N(k, "5")), block(k, FP(k, f, 0)), k.yes()));
           k.ens(f, k.cond(k.gt(FP(k, f, 1), N(k, "5")), block(k, FP(k, f, 0)), k.yes()));
           k.done(f);
       });

    // B3: int *same(int *p) requires(live(p)) ensures(r, live(r)) { return p; }   void peek(int *q) requires(live(q))
    // ensures(live(q)) {}
    //     void f() { int *p = malloc(); if (p != NULL) { int *q = same(p); peek(q); } }
    // the given live(r) may share its liveness cell with the caller's chunk: the call is followed by the case split on
    // the cells, never Silicon's own check (an exit 3 before)
    lc("pg_live_given", "the result of a callee that gives live(r) is used after the call: the liveness cells are split against the arguments",
       [=](Kit& k)
       {
           Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
           Function* same {k.fn("same", PT(k), {{"p", PT(k)}})};
           k.req(same, live(k, FP(k, same, 0), 0));
           k.ens(same, live(k, k.b.text(k.R(same), "r"), 1));
           same->body = k.blk({k.ret(k.pv(same, 0))});
           k.done(same);
           Function* peek {k.fn("peek", k.vd, {{"q", PT(k)}})};
           k.req(peek, live(k, FP(k, peek, 0), 0));
           k.ens(peek, live(k, FP(k, peek, 0), 0));
           peek->body = k.blk({});
           k.done(peek);
           Function* f {k.fn("f", k.vd)};
           Var* p {k.local(f, "p", PT(k))};
           Var* q {k.local(f, "q", PT(k))};
           f->body = k.blkc({k.decl(p, k.call(m, {})),
                             k.iff(k.ne(k.pn(p), k.nul(PT(k))),
                                   k.blk({k.decl(q, k.call(same, {k.pn(p)})), k.es(k.call(peek, {k.pn(q)}))}))});
           k.done(f);
           k.fail_only("leak", f->end_loc, 1U, 0U);
       });


    // S3: void touch(int *p) requires(owns_block(p)) ensures(owns_block(p)) { { owning int *v = p; *v = 1; } *p = 2; }
    // (a borrowed block parked in an owner of an inner block)
    // the formal outlives the inner block, so the owner's exit there is no leak; the twin: owning int *v = malloc() in
    // the same inner block (a leak at its end)
    const auto parked = [=](Kit& k, bool fresh) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* f {k.fn("touch", k.vd, {{"p", PT(k)}})};
        k.req(f, block(k, FP(k, f, 0)));
        k.ens(f, block(k, FP(k, f, 0)));
        Var* v {k.local(f, "v", PT(k))};
        v->owning = true;
        std::vector<const Stmt*> in {k.decl(v, fresh ? k.call(m, {}) : k.pv(f, 0))};
        if (!fresh)
        {
            in.push_back(k.store(k.pn(v), N(k, "1")));
        }
        const Stmt* inner {k.blkc(in)};
        f->body = k.blkc({inner, k.store(k.pv(f, 0), N(k, "2"))});
        k.done(f);
        return inner->end_loc;
    };
    lc("pe_parked_inner", "a borrowed block parked in an owning local of an inner block: the formal outlives the block's end (no leak there)",
       [=](Kit& k)
       {
           parked(k, false);
       });
    lc("pe_parked_inner_fresh", "an owner of a fresh block at the same inner block's end: a leak",
       [=](Kit& k)
       {
           k.fail("leak", parked(k, true));
       });

    // S2: void f() { owning int *v = NULL; if ((v = malloc()) == NULL) { return; } *v = 1; [free(v);] }     (an owner
    // assigned inside an expression: the rows, `held_v`, `tv_v`)
    const auto assigned_in_expr = [=](Kit& k, bool freed, bool cell) -> Built
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd)};
        Var* v {k.local(f, "v", PT(k))};
        v->owning = true;
        Var* x {cell ? k.cell(f, "x", k.i32) : nullptr};
        const Expr* as {k.asgE(v, cell ? k.addr(x) : k.call(m, {}))};
        std::vector<const Stmt*> top;
        if (cell)
        {
            top.push_back(k.decl(x, N(k, "0")));
        }
        top.push_back(k.decl(v, k.nul(PT(k))));
        top.push_back(k.iff(k.eq(as, k.nul(PT(k))), k.blk({k.ret(nullptr)})));
        top.push_back(k.store(k.pn(v), N(k, "1")));
        if (freed)
        {
            top.push_back(k.es(k.call(fr, {k.pn(v)})));
        }
        const Stmt* fb {k.blkc(top)};
        f->body = fb;
        k.done(f);
        return Built {as->loc, fb->end_loc};
    };
    lc("px_assign_expr", "`if ((v = malloc()) == NULL)` of an owning local: the value row holds (a held block), the block is freed after",
       [=](Kit& k)
       {
           assigned_in_expr(k, true, false);
       });
    lc("px_assign_expr_leak", "the same without the free: the owner still holds its block at the closing brace (leak)",
       [=](Kit& k)
       {
           k.fail("leak", assigned_in_expr(k, false, false).fn_end);
       });
    lc("px_assign_expr_cell", "`(v = &x) == NULL` of an owning local, x a cell: the value row fails (permission), the function holds no block there",
       [=](Kit& k)
       {
           k.fail("permission", assigned_in_expr(k, false, true).at);
       });

    // S1: int *where(int c) ensures(r, live(r)) { static int s = 5; if (c) { return &s; } return NULL; }    (live(r) is
    // covered by the implicit owns(s) on the first path only)
    // the pure rows (`r != NULL`) are guarded by the atom's own guard, not by `!cov`: the NULL path is a permission
    // error, never Silicon's own check
    lc("ps_cov_live_null", "a covered live(r) whose body returns NULL on one path: the pure row of the atom fails (permission) at that return",
       [=](Kit& k)
       {
           Function* f {k.fn("where", PT(k), {{"c", k.i32}})};
           Global* s {k.prog.add_global("s", k.i32, k.at(), f)};
           s->init = k.num(k.i32, "5");
           s->value = "5";
           k.req(f, k.b.global_atom(AtomKind::Owns, k.b.global_addr(s, k.at()), 0, k.at(), true));
           k.ens(f, k.b.global_atom(AtomKind::Owns, k.b.global_addr(s, k.at()), 0, k.at(), true));
           k.ens(f, k.b.atom(AtomKind::Live, k.b.text(k.R(f), "r"), 1, k.at()));
           const Stmt* none {k.ret(k.nul(PT(k)))};
           f->body = k.blk({k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({k.ret(k.b.global_addr(s, k.at()))})), none});
           k.done(f);
           // the atom's null row is an ending row placed first: one fault, one report
           k.fail_only("permission", none->loc, 8U, 0U);
       });

    // audit round 3 B1: void f() { owning int *a = malloc(); if (a == NULL) { return; } *a = 0; owning int *b = a; int
    // i = 0; while (i < 3) { *a = i; *b = i; [free(b);] i++; } free(b); }
    // two owners of one block carried by one loop: the later owner's carrier clauses are guarded by `block_of(b) !=
    // block_of(a)`; the twin frees through b in the body and keeps both
    // owners non-null (the back-edge row `v != null ==> held_v`: permission at the end of the body)
    const auto two_owners = [=](Kit& k, bool free_in_body) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd)};
        Var* a {k.local(f, "a", PT(k))};
        a->owning = true;
        Var* b {k.local(f, "b", PT(k))};
        b->owning = true;
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body {k.store(k.pn(a), k.v(i)), k.store(k.pn(b), k.v(i))};
        if (free_in_body)
        {
            body.push_back(k.es(k.call(fr, {k.pn(b)})));
        }
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* bs {k.blkc(body)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        f->body = k.blkc({k.decl(a, k.call(m, {})), k.iff(k.eq(k.pn(a), k.nul(PT(k))), k.blk({k.ret(nullptr)})),
                          k.store(k.pn(a), N(k, "0")), k.decl(b, k.pn(a)), k.decl(i, N(k, "0")), lp,
                          k.es(k.call(fr, {k.pn(b)}))});
        k.done(f);
        return bs->end_loc;
    };
    lc("olp_two_owners", "two owning locals of one block, both used in a loop: carried once, freed through b after it",
       [=](Kit& k)
       {
           two_owners(k, false);
       });
    lc("olp_two_owners_free_in_body",
       "the body frees through b and leaves both owners pointing at the freed block: the back-edge row of the first owner fails (permission at the end of the body)",
       [=](Kit& k)
       {
           k.fail_only("permission", two_owners(k, true), 14U, 0U);
       });

    // audit round 3 S1: void f() { owning int *v = malloc(); if (v == NULL) { return; } *v = 5; int i = 0; int t = 0;
    // while (i < 3) { t = *v; i++; } check(*v == 5); free(v); }
    // an owner the loop reads and does not assign is the variable-keyed heap place (value and flag kept), so its value
    // after the loop is the one before it
    lc("olp_read_keeps_value", "an owner only read in the loop keeps its value across it (`*v == 5` after the loop)",
       [=](Kit& k)
       {
           Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
           Function* fr {heap_free_view(k, k.i32)};
           Function* f {k.fn("f", k.vd)};
           Var* v {k.local(f, "v", PT(k))};
           v->owning = true;
           Var* i {k.local(f, "i", k.i32)};
           Var* t {k.local(f, "t", k.i32)};
           const Stmt* bs {k.blkc({k.asg(t, k.deref(k.pn(v))), k.asg(i, k.add(k.v(i), N(k, "1")))})};
           const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                 {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
           f->body = k.blkc({k.decl(v, k.call(m, {})), k.iff(k.eq(k.pn(v), k.nul(PT(k))), k.blk({k.ret(nullptr)})),
                             k.store(k.pn(v), N(k, "5")), k.decl(i, N(k, "0")), k.decl(t, N(k, "0")), lp,
                             k.chk(k.eq(k.deref(k.pn(v)), N(k, "5"))), k.es(k.call(fr, {k.pn(v)}))});
           k.done(f);
       });

    // audit 3b: a block that becomes shared inside the loop (the carriers were told apart by the entry state): a row at
    // the back edge, permission, never Silicon's own check
    // void f() { owning int *a = malloc(); if (a == NULL) { return; } *a = 0; owning int *b = NULL; int i = 0; while (i
    // < 3) { if (b == NULL) { b = a; } i++; } free(a); }
    // void g() { int *w = malloc(); if (w == NULL) { return; } *w = 0; owning int *v = NULL; int i = 0; while (i < 3) {
    // if (v == NULL) { v = w; } *w = i; i++; } free(v); }
    const auto shared = [=](Kit& k, bool via_owner) -> Built
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd)};
        Var* a {k.local(f, "a", PT(k))};
        a->owning = via_owner;
        Var* b {k.local(f, "b", PT(k))};
        b->owning = true;
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body;
        const Stmt* take {k.asg(b, k.pn(a))};
        body.push_back(k.iff(k.eq(k.pn(b), k.nul(PT(k))), k.blk({take})));
        if (!via_owner)
        {
            body.push_back(k.store(k.pn(a), k.v(i)));
        }
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* bs {k.blkc(body)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        const Stmt* fb {k.blkc({k.decl(a, k.call(m, {})), k.iff(k.eq(k.pn(a), k.nul(PT(k))), k.blk({k.ret(nullptr)})),
                                k.store(k.pn(a), N(k, "0")), k.decl(b, k.nul(PT(k))), k.decl(i, N(k, "0")), lp,
                                k.es(k.call(fr, {k.pn(via_owner ? a : b)}))})};
        f->body = fb;
        k.done(f);
        // the leak row after the end label, or the owner's exit row
        return Built {bs->end_loc, via_owner ? fb->end_loc : f->end_loc};
    };
    lc("olp_becomes_shared_owner", "an owner that was NULL at the entry takes another owner's block in the loop: the pair row at the back edge fails (permission)",
       [=](Kit& k)
       {
           const Built b {shared(k, true)};
           k.fail_only("permission", b.at, 10U, 3U);
           k.fail_only("leak", b.fn_end, 2U, 0U); // the exit row of b fails (a still holds the block b took)
       });
    lc("olp_becomes_shared_place", "an owner that was NULL at the entry takes the block of a non-owning pointer the loop uses: the pair row names the pointer (permission)",
       [=](Kit& k)
       {
           const Built b {shared(k, false)};
           k.fail_only("permission", b.at, 9U, 2U);
           k.fail("leak", b.fn_end);
       });
}

}
