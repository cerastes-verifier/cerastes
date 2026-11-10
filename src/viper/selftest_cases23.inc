// (included by selftest.cpp) Owning fields, the value rule (decision 0055 item 4, M4.5d2b2b): a pointer stored into an owning field
// (`StructMember::owning`) that is not null is the start of a block the function holds whole (`permission`, at the store); a null pointer
// constant needs no row and the row reads no old value.  The rule is at every owning leaf write: a store through a pointer or by name, a store inside
// an expression, a store through a nested member and the leaf writes of a whole-struct store.
namespace {

void define_owning_fields()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    // struct S { int *q (owning); int n; }   struct T { struct S s; int z; }
    auto mk_s = [](Kit& k)
    {
        StructType* S {k.strukt("S", {{"q", k.ptr(k.i32)}, {"n", k.i32}})};
        S->members[0U].owning = true;
        return S;
    };
    auto mk_t = [](Kit& k, StructType* s)
    {
        return k.strukt("T", {{"s", k.st(s)}, {"z", k.i32}});
    };
    auto owns_p = [](Kit& k, Function* f)
    {
        return k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at());
    };
    // `a` is live but its block is not held: `reads(*a)` in both lists (a use of the pointer is checked first; the value rule is the failure under test)
    auto reads_a = [](Kit& k, Function* f, AtomKind kind = AtomKind::Reads)
    {
        return k.b.atom(kind, k.b.text(k.F(f, 1), "a"), 1, k.at());
    };
    // void put(struct S *p, int *a) requires(owns(*p)) ensures(owns(*p)) { BODY }
    // `held`: int *t = malloc(); if (t != NULL) { *t = 1; STORE; free(t); }  with STORE = p->q = t
    // mode 0: p->q = t (t is a block the function holds)       mode 1: p->q = a (a parameter: no block of its own)
    // mode 2: p->q = NULL                                       mode 3: if ((p->q = t) != NULL) {}  (the store inside an expression)
    // mode 4: p->q = a with owns(*a) (the cells, not the block)  mode 5: p->q = &x (the address of a local)
    const auto put = [=](Kit& k, int mode) -> const Stmt*
    {
        StructType* S {mk_s(k)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("put", k.vd, {{"p", k.ptr(k.st(S))}, {"a", PI(k)}})};
        const AtomKind ka {mode == 4 ? AtomKind::Owns : AtomKind::Reads};
        k.req(f, k.land(owns_p(k, f), reads_a(k, f, ka)));
        k.ens(f, k.land(owns_p(k, f), reads_a(k, f, ka)));
        Var* t {k.local(f, "t", PI(k))};
        Var* x {k.cell(f, "x", k.i32)};
        const Expr* pv {k.pn(f->param_var(0))};
        const Stmt* st {nullptr};
        std::vector<const Stmt*> in {k.store(k.pn(t), N(k, "1"))};
        if (mode == 1)
        {
            st = k.lstore(pv, 0, k.pn(f->param_var(1)));
            in.push_back(st);
        }
        else if (mode == 2)
        {
            st = k.lstore(pv, 0, k.nul(PI(k)));
            in.push_back(st);
        }
        else if (mode == 3)
        {
            in.push_back(k.iff(k.ne(k.b.store_expr(k.mem(pv, 0), k.pn(t), k.at()), k.nul(PI(k))), k.blk({})));
            st = in.back();
        }
        else if (mode == 4)
        {
            st = k.lstore(pv, 0, k.pn(f->param_var(1)));
            in.push_back(st);
        }
        else if (mode == 5)
        {
            st = k.lstore(pv, 0, k.addr(x));
            in.push_back(st);
        }
        else
        {
            st = k.lstore(pv, 0, k.pn(t));
            in.push_back(st);
        }
        in.push_back(k.es(k.call(fr, {k.pn(t)})));
        std::vector<const Stmt*> body {k.decl(t, k.call(m, {})), k.decl(x, N(k, "0")),
                                       k.iff(k.ne(k.pn(t), k.nul(PI(k))), k.blk(in))};
        f->body = k.blkc(body);
        k.done(f);
        return st;
    };
    lc("owning_field_value_held", "p->q = t where t is a block the function holds whole: the value rule holds",
       [=](Kit& k)
       {
           put(k, 0);
       });
    lc("owning_field_value_param",
       "p->q = a with a a parameter of no atom: the function does not hold a's block (permission, at the store)",
       [=](Kit& k)
       {
           k.fail("permission", put(k, 1)->loc, 1U, "", "owning field");
       });
    lc("owning_field_value_null", "p->q = NULL: a null pointer constant needs no row",
       [=](Kit& k)
       {
           put(k, 2);
       });
    lc("owning_field_value_in_expr", "(p->q = t) != NULL as a condition: the value rule holds inside the expression",
       [=](Kit& k)
       {
           put(k, 3);
       });
    lc("owning_field_value_cells",
       "p->q = a with owns(*a) in the contract: the cells are held but not the block (permission, at the store)",
       [=](Kit& k)
       {
           k.fail("permission", put(k, 4)->loc, 1U, "", "owning field");
       });
    lc("owning_field_value_local", "p->q = &x with x a local: not the start of a block the function holds (permission)",
       [=](Kit& k)
       {
           k.fail("permission", put(k, 5)->loc, 1U, "", "owning field");
       });

    // void put2(struct T *p) requires(owns(*p)) ensures(owns(*p)) { int *t = malloc(); if (t != NULL) { *t = 1; [p->s.q = t | p->s.q = a]; free(t); } }
    const auto nested = [=](Kit& k, bool bad) -> const Stmt*
    {
        StructType* S {mk_s(k)};
        StructType* T {mk_t(k, S)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("put2", k.vd, {{"p", k.ptr(k.st(T))}, {"a", PI(k)}})};
        k.req(f, k.land(owns_p(k, f), reads_a(k, f)));
        k.ens(f, k.land(owns_p(k, f), reads_a(k, f)));
        Var* t {k.local(f, "t", PI(k))};
        const Expr* pv {k.pn(f->param_var(0))};
        const Stmt* st {k.store(k.mem(k.mem(pv, 0), 0), bad ? k.pn(f->param_var(1)) : k.pn(t))};
        std::vector<const Stmt*> in {k.store(k.pn(t), N(k, "1")), st, k.es(k.call(fr, {k.pn(t)}))};
        f->body = k.blkc({k.decl(t, k.call(m, {})), k.iff(k.ne(k.pn(t), k.nul(PI(k))), k.blk(in))});
        k.done(f);
        return st;
    };
    lc("owning_field_value_nested", "p->s.q = t through a nested member: the value rule holds",
       [=](Kit& k)
       {
           nested(k, false);
       });
    lc("owning_field_value_nested_bad", "p->s.q = a through a nested member: permission, at the store",
       [=](Kit& k)
       {
           k.fail("permission", nested(k, true)->loc, 1U, "", "owning field");
       });

    // void whole(struct S *p, int *a) requires(owns(*p)) ensures(owns(*p)) { int *t = malloc(); if (t != NULL) { *t = 1; *p = (struct S){ [t | a], 0 }; free(t); } }: the
    // whole-struct store writes the owning leaf, so the value rule applies to the aggregate's element.
    const auto whole = [=](Kit& k, bool bad) -> const Stmt*
    {
        StructType* S {mk_s(k)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("whole", k.vd, {{"p", k.ptr(k.st(S))}, {"a", PI(k)}})};
        k.req(f, k.land(owns_p(k, f), reads_a(k, f)));
        k.ens(f, k.land(owns_p(k, f), reads_a(k, f)));
        Var* t {k.local(f, "t", PI(k))};
        const Expr* pv {k.pn(f->param_var(0))};
        const Expr* q {bad ? k.pn(f->param_var(1)) : k.pn(t)};
        const Stmt* st {k.store(pv, k.b.aggregate(k.st(S), {q, N(k, "0")}, k.at()))};
        std::vector<const Stmt*> in {k.store(k.pn(t), N(k, "1")), st, k.es(k.call(fr, {k.pn(t)}))};
        f->body = k.blkc({k.decl(t, k.call(m, {})), k.iff(k.ne(k.pn(t), k.nul(PI(k))), k.blk(in))});
        k.done(f);
        return st;
    };
    lc("owning_field_value_whole",
       "*p = (struct S){ t, 0 } with t a block the function holds: the whole-struct store's value rule holds",
       [=](Kit& k)
       {
           whole(k, false);
       });
    lc("owning_field_value_whole_bad", "*p = (struct S){ a, 0 } with a a parameter: permission, at the store",
       [=](Kit& k)
       {
           k.fail("permission", whole(k, true)->loc, 1U, "", "owning field");
       });

    // ---- rejections of check_program (item 2): the owning-field graph ----
    rc("owning_field_non_pointer", "is not a pointer",
       [=](Kit& k)
       {
           StructType* S {k.strukt("S", {{"n", k.i32}})};
           S->members[0U].owning = true;
       });
    rc("owning_field_cycle_self", "makes the struct own itself",
       [=](Kit& k)
       {
           StructType* N {k.prog.add_struct("node", "struct node", k.at())};
           k.prog.add_member(N, "next", k.ptr(k.st(N)), k.at());
           N->members[0U].owning = true;
       });
    rc("owning_field_cycle_pair", "makes the struct own itself",
       [=](Kit& k)
       {
           StructType* A {k.prog.add_struct("A", "struct A", k.at())};
           StructType* B {k.prog.add_struct("B", "struct B", k.at())};
           k.prog.add_member(A, "b", k.ptr(k.st(B)), k.at());
           k.prog.add_member(B, "a", k.ptr(k.st(A)), k.at());
           A->members[0U].owning = true;
           B->members[0U].owning = true;
       });
    // A `static` struct local's member that is not owning is still not readable in a contract (decision 0041 item 12): only an owning leaf is, by the closure clause of its
    // implicit `owns(s)` (decision 0055 item 3).
    rc("owning_field_static_plain_member_read", "a read of the `static` local",
       [=](Kit& k)
       {
           StructType* S {mk_s(k)};
           Function* f {k.fn("f", k.vd)};
           Global* g {k.prog.add_global("origin", k.st(S), k.at(), f)};
           for (const char* v : {"0", "0"})
           {
               GlobalLeaf l;
               l.kind = Global::Init::Value;
               l.init = N(k, v);
               l.value = v;
               l.loc = k.at();
               g->leaves.push_back(l);
           }
           k.ens(f, k.eq(k.leaf(k.b.global_addr(g, k.at()), 1), N(k, "0")));
           f->body = k.blk({});
           k.done(f);
       });
}

} // namespace
