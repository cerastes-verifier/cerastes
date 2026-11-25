// (included by selftest.cpp) M4.5e1e2 Part A (record 0064 item 4): the guards behind the refusals of arrays of structures and arrays of arrays.  Each `ru` case lowers a program that
// check_program refuses and expects the guard's internal error (lowering must never read an aggregate element as a cell or a row as a flat pointer); each `rc` case is a refusal of
// check_program that Part A closes.  A part that later gives a site its rows removes that site's guard and keeps its case as a positive one.
namespace {

void define_aggregate_element_guards()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // struct T { int x; }; struct S { struct T a[2]; int n; }
    auto S_agg = [](Kit& k)
    {
        StructType* T {k.strukt("T", {{"x", k.i32}})};
        return k.strukt("S", {{"a", k.prog.array_of(k.st(T), 2U)}, {"n", k.i32}});
    };
    // int g[2][3];
    auto matrix = [](Kit& k)
    {
        return k.prog.add_global("g", k.prog.array_of(k.prog.array_of(k.i32, 3U), 2U), k.at(), nullptr);
    };
    // g[1][2]
    auto cell_of_matrix = [](Kit& k, const Global* g, const char* i, const char* j)
    {
        const Expr* row {k.b.elem_addr(k.b.global_addr(g, k.at()), k.num(k.i32, i), k.at())};
        return k.deref(k.b.elem_addr(row, k.num(k.i32, j), k.at()));
    };

    // ---- the views of a structure's leaves (vir.h, StructLeaf::aggregate_element) ----
    ru("e1e2a_guard_object_view", "reached a walk of a structure's leaves that has no rows for it",
       [=](Kit& k)
       {
           // void f(void) { struct S s; }  with struct S { struct T a[2]; int n; }
           StructType* S {S_agg(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           f->body = k.blkc({k.decl(s, nullptr)});
           k.done(f);
       });
    ru("e1e2a_guard_value_view", "reached a walk of a structure's leaves that has no rows for it",
       [=](Kit& k)
       {
           // void f(struct S s) {}
           StructType* S {S_agg(k)};
           Function* f {k.fn("f", k.vd, {{"s", k.st(S)}})};
           f->body = k.blk({});
           k.done(f);
       });

    // ---- trap 1: an element of a row is not a flat element of a pointer ----
    ru("e1e2a_guard_elem_addr_of_a_row", "the address of a row of an array of arrays",
       [=](Kit& k)
       {
           // int g[2][3]; int f(void) { return g[1][2]; }
           Global* g {matrix(k)};
           Function* f {k.fn("f", k.i32)};
           f->body = k.blk({k.ret(cell_of_matrix(k, g, "1", "2"))});
           k.done(f);
       });
    ru("e1e2a_guard_elem_addr_of_a_row_parameter", "the address of a row of an array of arrays",
       [=](Kit& k)
       {
           // int f(int (*p)[3]) { return p[0][1]; }  (a pointer to an array: the row's address is no flat pointer)
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.prog.array_of(k.i32, 3U))}})};
           const Expr* row {k.b.elem_addr(k.F(f, 0), N(k, "0"), k.at())};
           f->body = k.blk({k.ret(k.deref(k.b.elem_addr(row, N(k, "1"), k.at())))});
           k.done(f);
       });
    ru("e1e2a_guard_inv_elem_of_a_row", "the address of a row of an array of arrays",
       [=](Kit& k)
       {
           // int g[2][3]; void f(void) { int i = 0; while (i < 3) invariant(0 <= i && i <= 3 && g[1][2] >= 0) decreases(3 - i) { i = i + 1; } }
           Global* g {matrix(k)};
           Function* f {k.fn("f", k.vd)};
           Var* i {k.local(f, "i", k.i32)};
           const Expr* rd {cell_of_matrix(k, g, "1", "2")};
           const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "3")),
                                   {k.cl(k.range(k.v(i), k.i32, "0", "3")), k.cl(k.ge_c(rd, k.i32, "0"))},
                                   {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({k.asg(i, k.add(k.v(i), N(k, "1")))}))};
           f->body = k.blkc({k.decl(i, N(k, "0")), loop});
           k.done(f);
       });

    // ---- trap 2: element keys of a base that holds an index of its own ----
    ru("e1e2a_guard_elem_key_of_a_row", "(same_elem_base; decision 0064 item 4",
       [=](Kit& k)
       {
           // void f(int (*p)[3]) requires(p[0][0] == p[1][0]) {}
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.prog.array_of(k.i32, 3U))}})};
           auto at = [&](const char* i)
           {
               const Expr* row {k.b.elem_addr(k.F(f, 0), N(k, i), k.at())};
               return k.deref(k.b.elem_addr(row, N(k, "0"), k.at()));
           };
           k.req(f, k.eq(at("0"), at("1")));
           f->body = k.blk({});
           k.done(f);
       });

    ru("e1e2a_guard_range_over_structures_in_a_call",
       "a range over structures or arrays reached a site that names one field for it",
       [=](Kit& k)
       {
           // void g(struct T *p, int n) requires(owns_range(p, n)); void f(struct T *p, int n) requires(owns_range(p, n)) { g(p, n); }
           StructType* T {k.strukt("T", {{"x", k.i32}})};
           Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(T))}, {"n", k.i32}})};
           k.req(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
           k.trusted(g);
           k.done(g);
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}, {"n", k.i32}})};
           k.req(f, k.b.range_atom(AtomKind::OwnsRange, k.F(f, 0), k.F(f, 1), 0, k.at()));
           f->body = k.blk({k.es(k.call(g, {k.pv(f, 0), k.pv(f, 1)}))});
           k.done(f);
       });

    // ---- trap 3: a range over structures, and an array that names no field ----
    ru("e1e2a_guard_range_over_structures", "a range over structures or arrays reached the range of one element type",
       [=](Kit& k)
       {
           // void f(struct T *p, int n) requires(owns_range(p, n)) {}
           StructType* T {k.strukt("T", {{"x", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}, {"n", k.i32}})};
           k.req(f, k.b.range_atom(AtomKind::OwnsRange, k.F(f, 0), k.F(f, 1), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    // Part B lifts this guard: an array of structures declared and never used is allocated and left like any other (flipped from `ru`)
    lc("e1e2a_guard_array_of_structures_alloc",
       "an array of structures declared and never used: its allocation and its scope exit",
       [=](Kit& k)
       {
           // void f(void) { struct T v[2]; }
           StructType* T {k.strukt("T", {{"x", k.i32}})};
           Function* f {k.fn("f", k.vd)};
           Var* v {f->new_local("v", k.prog.array_of(k.st(T), 2U), k.at())};
           f->mark_address_taken(v);
           f->body = k.blkc({k.decl(v, nullptr)});
           k.done(f);
       });

    // ---- the backstop gaps of check.cpp that Part A closes (the encoder refused these; the checker did not) ----
    // struct U { int a[3]; int n; }
    auto U_arr = [](Kit& k)
    {
        return k.strukt("U", {{"a", k.prog.array_of(k.i32, 3U)}, {"n", k.i32}});
    };
    rc("e1e2a_rej_invariant_owns_of_a_structure_with_an_array_member",
       "a permission atom on a structure with an array member in a loop invariant",
       [=](Kit& k)
       {
           // void f(struct U *p) requires(owns(*p)) ensures(owns(*p)) { struct U *q = p; int i = 0; while (i < 3) invariant(owns(*q) && 0 <= i && i <= 3) decreases(3 - i) { i = i + 1; } }
           StructType* U {U_arr(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(U))}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           Var* q {k.local(f, "q", k.ptr(k.st(U)))};
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* loop {
               k.whl(k.lt(k.v(i), N(k, "3")),
                     {k.cl(k.b.atom(AtomKind::Owns, k.v(q), 1, k.at())), k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                     {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({k.asg(i, k.add(k.v(i), N(k, "1")))}))};
           f->body = k.blkc({k.decl(q, k.F(f, 0)), k.decl(i, N(k, "0")), loop});
           k.done(f);
       });
    rc("e1e2a_rej_partnerless_ensures_of_a_structure_with_an_array_member",
       "an `ensures` atom without a `requires` atom on a structure with an array member or on an array member",
       [=](Kit& k)
       {
           // struct U *f(void) ensures(owns(*result)): an atom the function creates, on a structure with an array member
           StructType* U {U_arr(k)};
           Function* f {k.fn("f", k.ptr(k.st(U)))};
           k.trusted(f);
           k.ens(f, k.b.atom(AtomKind::Owns, k.R(f), 0, k.at()));
           k.done(f);
       });
    rc("e1e2a_rej_owns_of_a_structure_with_an_array_member_through_a_heap_read",
       "a permission atom on a structure with an array member of a place reached through a heap read",
       [=](Kit& k)
       {
           // struct R { struct U *q; }; void f(struct R *r) requires(owns(*r->q))
           StructType* U {U_arr(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(U))}})};
           Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.deref(k.mem(k.F(f, 0), 0)), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });

    // ---- audit round 1, issue 2: `restrict` beside a parameter that holds the cell only inside an aggregate element (the call at check.cpp's restrict rule; the shapes are refused for
    // other reasons today, so the case matches on the `restrict` message among the diagnostics) ----
    rc("e1e2a_rej_restrict_beside_an_array_of_structures", "is `restrict` without",
       [=](Kit& k)
       {
           // int f(int *restrict p, struct S *q) requires(owns(*p)) ensures(owns(*p)) with struct S { struct T a[2]; int n; }: q may hold the cell p points to, in a[].x
           StructType* S {S_agg(k)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.i32)}, {"q", k.ptr(k.st(S))}})};
           f->params[0U].is_restrict = true;
           k.req(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });

    // ---- trap 5: the nest walkers of the checker descend into elements ----
    rc("e1e2a_rej_by_value_cycle_in_an_array", "contains the struct itself by value",
       [=](Kit& k)
       {
           // struct A { struct B b[2]; }; struct B { struct A a; };
           StructType* A {k.strukt("A", {{"x", k.i32}})};
           StructType* B {k.strukt("B", {{"a", k.st(A)}})};
           k.prog.add_member(A, "b", k.prog.array_of(k.st(B), 2U), k.at());
       });
    rc("e1e2a_rej_marked_member_in_an_array_element", "has an array member and an `owning` or `ref_to_uninit` member",
       [=](Kit& k)
       {
           // struct T { int *owning p; }; struct S { struct T a[2]; int n[2]; }
           StructType* T {k.strukt("T", {{"p", k.ptr(k.i32)}})};
           T->members[0U].owning = true;
           k.strukt("S", {{"a", k.prog.array_of(k.st(T), 2U)}, {"n", k.prog.array_of(k.i32, 2U)}});
       });
}

// The leaf paths of record 0064 item 4, the flag of the three views and the spelling of the new places, checked directly (they have no Viper): returns the number of failures.
int check_leaf_paths()
{
    int bad {0};
    auto expect = [&](bool ok, const std::string& what)
    {
        if (!ok)
        {
            std::fprintf(stderr, "FAIL (leaf paths) %s\n", what.c_str());
            ++bad;
        }
    };
    Kit k;
    // for a type that decision 0063 accepted the paths are struct_object_leaves' list: struct T { int p; int a[2]; }; struct S { struct T t; int *q[2]; bool b; };
    StructType* T {k.strukt("T", {{"p", k.i32}, {"a", k.prog.array_of(k.i32, 2U)}})};
    StructType* S {k.strukt("S", {{"t", k.st(T)}, {"q", k.prog.array_of(k.ptr(k.i32), 2U)}, {"b", k.bl}})};
    const std::vector<StructLeaf> leaves {struct_object_leaves(*S, "s")};
    const std::vector<LeafPath> paths {object_leaf_paths(k.st(S), "s")};
    bool same {leaves.size() == paths.size()};
    for (size_t i {0U}; same && i < leaves.size(); ++i)
    {
        const StructLeaf& l {leaves[i]};
        const LeafPath& p {paths[i]};
        same = l.path == p.members() && l.text == p.text && (l.array ? l.element : l.type) == p.type &&
               (l.array ? p.indexes() == 1U && p.cells() == l.count && p.steps.back().kind == LeafStep::Kind::Index
                        : p.indexes() == 0U && p.cells() == 1U) &&
               l.owning == p.owning && l.ref_to_uninit == p.ref_to_uninit;
    }
    expect(same, "object_leaf_paths of S is struct_object_leaves' list (paths, texts, cell types, counts)");
    // a scalar is one path with no steps
    expect(object_leaf_paths(k.i32).size() == 1U && object_leaf_paths(k.i32)[0U].steps.empty(),
           "a scalar is one path without steps");
    // struct V { int x; int a[2]; }; V v[3]: one path per leaf, the index step first: v[].x (3 cells) and v[].a (3 * 2 cells)
    StructType* V {k.strukt("V", {{"x", k.i32}, {"a", k.prog.array_of(k.i32, 2U)}})};
    const std::vector<LeafPath> vs {object_leaf_paths(k.prog.array_of(k.st(V), 3U), "v")};
    expect(vs.size() == 2U, "an array of structures: one path per leaf of the element, whatever the count");
    if (vs.size() == 2U)
    {
        expect(vs[0U].text == "v[].x" && vs[0U].indexes() == 1U && vs[0U].cells() == 3U &&
                   vs[0U].members() == std::vector<int> {0} &&
                   vs[0U].steps.front() == LeafStep {LeafStep::Kind::Index, 0, 3U},
               "the path of v[].x: Index(3), Member(0)");
        expect(vs[1U].text == "v[].a" && vs[1U].indexes() == 2U && vs[1U].cells() == 6U && vs[1U].type == k.i32 &&
                   vs[1U].steps.size() == 3U,
               "the path of v[].a: Index(3), Member(1), Index(2), 6 cells");
    }
    // int m[2][3]: one path, two index steps, 6 cells, the text `m[]` (an index step followed by another is written, the last is not)
    const std::vector<LeafPath> ms {object_leaf_paths(k.prog.array_of(k.prog.array_of(k.i32, 3U), 2U), "m")};
    expect(ms.size() == 1U && ms[0U].indexes() == 2U && ms[0U].cells() == 6U && ms[0U].text == "m[]" &&
               ms[0U].type == k.i32,
           "a matrix is one path with two index steps");
    // a structure that holds an array of structures: the three views flag the member and do not expand it
    StructType* W {k.strukt("W", {{"v", k.prog.array_of(k.st(V), 2U)}, {"n", k.i32}})};
    for (const std::vector<StructLeaf>& view : {struct_leaves(*W), struct_object_leaves(*W), struct_value_leaves(*W)})
    {
        expect(
            view.size() == 2U && view[0U].aggregate_element && view[0U].type.is_array() && !view[1U].aggregate_element,
            "a view of a structure with an array of structures has the flagged member and the cell n, and no expansion");
    }
    expect(struct_object_leaves(*W)[0U].array && struct_object_leaves(*W)[0U].element == k.st(V),
           "the object view keeps `array` and the aggregate element");
    // a matrix member is flagged too
    StructType* X {k.strukt("X", {{"m", k.prog.array_of(k.prog.array_of(k.i32, 3U), 2U)}})};
    expect(struct_value_leaves(*X).size() == 1U && struct_value_leaves(*X)[0U].aggregate_element,
           "a matrix member is one flagged leaf");
    // the structures 0063 accepted are not flagged
    bool none {true};
    for (const StructLeaf& l : struct_value_leaves(*S))
    {
        none = none && !l.aggregate_element;
    }
    expect(none, "no leaf of S is flagged");

    // the spelling of the new places: g[i].x, g[i].a[j], g[i][j], p[i].x, v[i].t.x
    Global* gv {k.prog.add_global("gv", k.prog.array_of(k.st(V), 3U), k.at(), nullptr)};
    Global* gm {k.prog.add_global("gm", k.prog.array_of(k.prog.array_of(k.i32, 3U), 2U), k.at(), nullptr)};
    const Expr* i {k.b.text(k.num(k.i32, "0"), "i")};
    const Expr* j {k.b.text(k.num(k.i32, "0"), "j")};
    const Expr* gv_i {k.b.elem_addr(k.b.global_addr(gv, k.at()), i, k.at())};
    expect(gv_i->type == k.ptr(k.st(V)), "gv[i] is a pointer to the element structure");
    expect(place_text(*gv_i) == "gv[i]", "place_text of gv[i]");
    const Expr* gv_i_x {k.b.member_addr(gv_i, 0, k.at())};
    expect(place_text(*gv_i_x) == "gv[i].x" && gv_i_x->c_text == "&gv[i].x", "place_text and c_text of gv[i].x");
    const Expr* gv_i_a {k.b.member_addr(gv_i, 1, k.at())};
    expect(place_text(*k.b.elem_addr(gv_i_a, j, k.at())) == "gv[i].a[j]", "place_text of gv[i].a[j]");
    const Expr* gm_i {k.b.elem_addr(k.b.global_addr(gm, k.at()), i, k.at())};
    expect(gm_i->type == k.ptr(k.prog.array_of(k.i32, 3U)), "gm[i] is a pointer to the row");
    expect(place_text(*k.b.elem_addr(gm_i, j, k.at())) == "gm[i][j]", "place_text of gm[i][j]");
    Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(V))}})};
    k.done(f);
    const Expr* p_i {k.b.elem_addr(k.b.text(k.pv(f, 0), "p"), i, k.at())};
    expect(place_text(*k.b.member_addr(p_i, 0, k.at())) == "p[i].x", "place_text of p[i].x");
    // an element address that is a pointer value keeps its old spelling
    expect(k.b.member_addr(k.b.elem_addr(k.b.text(k.pv(f, 0), "p"), i, k.at()), 0, k.at())->c_text == "&p[i].x",
           "c_text of &p[i].x");
    return bad;
}

// Audit round 1, issue 2 (decision 0064 item 4): `restrict_backed`'s walk of the other parameters' pointees is by leaf path.  A parameter whose pointee holds an `int` cell only inside an array of
// structures, a matrix member or a row (a pointer to an array) reaches an `int *restrict p`, so it needs an atom of its own like any other `int` holder; read as one leaf of the aggregate's type,
// it never reached `p` and 0043 item 10 stopped asking for it (the unsound direction).  Checked directly: every shape is refused by check_program elsewhere today, so there is no case to run.
int check_restrict_reach()
{
    int bad {0};
    auto expect = [&](bool ok, const std::string& what)
    {
        if (!ok)
        {
            std::fprintf(stderr, "FAIL (restrict reach) %s\n", what.c_str());
            ++bad;
        }
    };
    // `int f(int *restrict p, <q's type> q) requires(owns(*p) [&& owns(*q)]) ensures(owns(*p) [&& owns(*q)])`: whether `restrict` on p is backed
    auto backed = [](Kit& k, const Type& qt, bool own_q)
    {
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.i32)}, {"q", qt}})};
        f->params[0U].is_restrict = true;
        const Expr* req {k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at())};
        const Expr* ens {k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at())};
        if (own_q)
        {
            req = k.land(req, k.b.atom(AtomKind::Owns, k.F(f, 1), 1, k.at()));
            ens = k.land(ens, k.b.atom(AtomKind::Owns, k.F(f, 1), 1, k.at()));
        }
        k.req(f, req);
        k.ens(f, ens);
        f->body = k.blk({k.ret(k.num(k.i32, "0"))});
        k.done(f);
        return restrict_backed(*f, 0);
    };
    {
        // struct P { int x; }; struct S { struct P v[2]; long n; }: the `int` is in `v[].x`
        Kit k;
        StructType* P {k.strukt("P", {{"x", k.i32}})};
        StructType* S {k.strukt("S", {{"v", k.prog.array_of(k.st(P), 2U)}, {"n", k.i64}})};
        expect(!backed(k, k.ptr(k.st(S)), false),
               "a structure whose `int` cell is inside an array of structures reaches `int *restrict p`");
        Kit k2;
        StructType* P2 {k2.strukt("P", {{"x", k2.i32}})};
        StructType* S2 {k2.strukt("S", {{"v", k2.prog.array_of(k2.st(P2), 2U)}, {"n", k2.i64}})};
        expect(backed(k2, k2.ptr(k2.st(S2)), true), "the same, with an atom of its own on the other parameter: backed");
    }
    {
        // struct M { int m[2][3]; }: the `int` is in a matrix member
        Kit k;
        StructType* M {k.strukt("M", {{"m", k.prog.array_of(k.prog.array_of(k.i32, 3U), 2U)}})};
        expect(!backed(k, k.ptr(k.st(M)), false),
               "a structure whose `int` cell is inside a matrix member reaches `int *restrict p`");
    }
    {
        // int (*q)[3]: a pointer to a row of `int`
        Kit k;
        expect(!backed(k, k.ptr(k.prog.array_of(k.i32, 3U)), false),
               "a pointer to a row of `int` reaches `int *restrict p`");
    }
    {
        // the controls: a structure with no `int` leaf anywhere does not reach p; an array of scalars member does (0063's types, unchanged)
        Kit k;
        StructType* Q {k.strukt("Q", {{"c", k.i64}})};
        StructType* R {k.strukt("R", {{"v", k.prog.array_of(k.st(Q), 2U)}})};
        expect(backed(k, k.ptr(k.st(R)), false),
               "a structure whose only leaf is a `long` cannot hold the `int` cell, in an array of structures too");
        Kit k2;
        StructType* U {k2.strukt("U", {{"a", k2.prog.array_of(k2.i32, 3U)}, {"n", k2.i32}})};
        expect(!backed(k2, k2.ptr(k2.st(U)), false),
               "an array member of `int` reaches `int *restrict p` (decision 0063, unchanged)");
    }
    return bad;
}

// ---- decision 0064 item 19: what the body of a function with a `restrict` parameter can reach (restrict_body_reason; question 326) ----
void define_restrict_body_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto glob = [](Kit& k, const char* name) -> Global*
    {
        Global* g {k.prog.add_global(name, k.i32, k.at(), nullptr)};
        g->init = k.num(k.i32, "0");
        g->value = "0";
        return g;
    };
    auto ga = [](Kit& k, const Global* g)
    {
        return k.b.global_addr(g, k.at());
    };
    auto gatom = [](Kit& k, AtomKind kind, const Expr* place, int amount)
    {
        return k.b.global_atom(kind, place, amount, k.at(), false);
    };
    // int f(int *restrict p) requires(owns(*p) [&& extra]) ensures(owns(*p) [&& extra]); the body is the caller's
    auto restrict_fn = [](Kit& k, const std::vector<std::pair<AtomKind, const Expr*>>& extra, bool guarded = false)
    {
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.i32)}, {"c", k.i32}})};
        f->params[0U].is_restrict = true;
        const Expr* req {k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at())};
        const Expr* ens {k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at())};
        int amount {1};
        for (const auto& [kind, place] : extra)
        {
            const Expr* r {k.b.global_atom(kind, place, amount, k.at(), false)};
            const Expr* e {k.b.global_atom(kind, place, amount, k.at(), false)};
            if (guarded)
            {
                r = k.lor(k.eq_c(k.F(f, 1), k.i32, "0"), r);
                e = k.lor(k.eq_c(k.F(f, 1), k.i32, "0"), e);
            }
            req = k.land(req, r);
            ens = k.land(ens, e);
            ++amount;
        }
        k.req(f, req);
        k.ens(f, ens);
        return f;
    };

    // (i) the body names the global: `*p = 1; g = 2; return *p;`
    rc("e1e2r_rej_body_names_a_global", "the body names or takes the address of the global `g`",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* f {restrict_fn(k, {})};
           f->body = k.blk({k.store(k.pv(f, 0), N(k, "1")), k.store(ga(k, g), N(k, "2")), k.ret(k.deref(k.pv(f, 0)))});
           k.done(f);
       });
    // (i) the body takes `&g` into a local pointer: `int *q = &g; return 0;`
    rc("e1e2r_rej_body_takes_the_address_of_a_global", "the body names or takes the address of the global `g`",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* f {restrict_fn(k, {})};
           Var* q {k.local(f, "q", k.ptr(k.i32))};
           f->body = k.blkc({k.decl(q, ga(k, g)), k.ret(N(k, "0"))});
           k.done(f);
       });
    // (i) a loop invariant that names the global counts as naming it
    rc("e1e2r_rej_loop_invariant_names_a_global", "the body names or takes the address of the global `g`",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* f {restrict_fn(k, {})};
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* loop {
               k.whl(k.lt(k.v(i), N(k, "3")),
                     {k.cl(k.range(k.v(i), k.i32, "0", "3")), k.cl(k.ge_c(k.deref(ga(k, g)), k.i32, "-5"))},
                     {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({k.asg(i, k.add(k.v(i), N(k, "1")))}))};
           f->body = k.blkc({k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
           k.done(f);
       });
    // (ii) a call of a function whose `requires` and `ensures` own `g`: `h()`
    rc("e1e2r_rej_callee_owns_a_global", "it calls `h`, whose contract has an atom on the global `g`",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* h {k.fn("h", k.vd)};
           k.req(h, gatom(k, AtomKind::Owns, ga(k, g), 0));
           k.ens(h, gatom(k, AtomKind::Owns, ga(k, g), 0));
           k.trusted(h);
           k.done(h);
           Function* f {restrict_fn(k, {})};
           f->body = k.blk({k.es(k.call(h, {})), k.ret(N(k, "0"))});
           k.done(f);
       });
    // (iii) a call of `int *getp(void)`: the result may point to the cell `*p` is
    rc("e1e2r_rej_callee_returns_a_pointer", "it calls `getp`, which returns a `int*`",
       [=](Kit& k)
       {
           Function* h {k.fn("getp", k.ptr(k.i32))};
           k.trusted(h);
           k.done(h);
           Function* f {restrict_fn(k, {})};
           Var* q {k.local(f, "q", k.ptr(k.i32))};
           f->body = k.blkc({k.decl(q, k.call(h, {})), k.ret(N(k, "0"))});
           k.done(f);
       });
    // (iii) a call of `void getp(int **out)`: it can hand a pointer back through the out-parameter
    rc("e1e2r_rej_callee_has_an_out_parameter", "it calls `getp`, whose parameter 1 is a `int**`",
       [=](Kit& k)
       {
           Function* h {k.fn("getp", k.vd, {{"out", k.ptr(k.ptr(k.i32))}})};
           k.trusted(h);
           k.done(h);
           Function* f {restrict_fn(k, {})};
           Var* q {k.local(f, "q", k.ptr(k.i32))};
           f->mark_address_taken(q);
           f->body = k.blkc({k.decl(q, k.nul(k.ptr(k.i32))), k.es(k.call(h, {k.addr(q)})), k.ret(N(k, "0"))});
           k.done(f);
       });
    // (iii) a call of `struct H geth(void)` with `struct H { int *q; }`: a structure result with a pointer leaf
    rc("e1e2r_rej_callee_returns_a_structure_with_a_pointer", "it calls `geth`, which returns a",
       [=](Kit& k)
       {
           StructType* H {k.strukt("H", {{"q", k.ptr(k.i32)}})};
           Function* h {k.fn("geth", k.st(H))};
           k.trusted(h);
           k.done(h);
           Function* f {restrict_fn(k, {})};
           f->body = k.blk({k.es(k.call(h, {})), k.ret(N(k, "0"))});
           k.done(f);
       });
    // (ii) an application of a `pure` function whose `requires` reads `g` is scanned like a call: `check(rd() >= 0)` (decision 0064 item 19, the audit's addition)
    rc("e1e2r_rej_apply_of_a_pure_function_reading_a_global",
       "it calls `rd`, whose contract has an atom on the global `g`",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* rd {k.fn("rd", k.i32)};
           rd->pure = true;
           rd->pure_body = N(k, "0");
           k.req(rd, gatom(k, AtomKind::Reads, ga(k, g), 0));
           k.done(rd);
           Function* f {restrict_fn(k, {})};
           f->body = k.blk({k.chk(k.ge_c(k.b.apply(rd, {}, "rd()", k.at()), k.i32, "0")), k.ret(N(k, "0"))});
           k.done(f);
       });
    // exempt forms: the function's own contract holds `g` whole and unguarded (a top-level conjunct of the `requires`); the body writes it and calls a function that owns it
    lc("e1e2r_owns_global_is_exempt", "restrict beside a written unguarded owns(g): the branch p == &g is dead",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* f {restrict_fn(k, {{AtomKind::Owns, ga(k, g)}})};
           f->body = k.blk({k.store(k.pv(f, 0), N(k, "1")), k.store(ga(k, g), N(k, "2")), k.ret(k.deref(k.pv(f, 0)))});
           k.done(f);
       });
    lc("e1e2r_reads_global_is_exempt",
       "restrict beside a written unguarded reads(g): the global is read and *p written",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* f {restrict_fn(k, {{AtomKind::Reads, ga(k, g)}})};
           f->body = k.blk({k.store(k.pv(f, 0), N(k, "1")), k.ret(k.deref(ga(k, g)))});
           k.done(f);
       });
    // a partial atom exempts nothing: struct G { int x; int y; } g; owns(g.x) while the body writes g.y
    rc("e1e2r_rej_partial_atom_does_not_exempt", "the body names or takes the address of the global `g`",
       [=](Kit& k)
       {
           StructType* G {k.strukt("G", {{"x", k.i32}, {"y", k.i32}})};
           Global* g {k.prog.add_global("g", k.st(G), k.at(), nullptr)};
           for (const char* v : {"0", "0"})
           {
               GlobalLeaf l;
               l.kind = Global::Init::Value;
               l.init = k.num(k.i32, v);
               l.value = v;
               l.loc = k.at();
               g->leaves.push_back(l);
           }
           Function* f {restrict_fn(k, {{AtomKind::Owns, k.b.member_addr(ga(k, g), 0, k.at())}})};
           f->body = k.blk({k.store(k.b.member_addr(ga(k, g), 1, k.at()), N(k, "2")), k.ret(N(k, "0"))});
           k.done(f);
       });
    // a guarded atom exempts nothing: `c == 0 || owns(g)`
    rc("e1e2r_rej_guarded_atom_does_not_exempt", "the body names or takes the address of the global `g`",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* f {restrict_fn(k, {{AtomKind::Owns, ga(k, g)}}, true)};
           f->body = k.blk({k.store(ga(k, g), N(k, "2")), k.ret(N(k, "0"))});
           k.done(f);
       });
    // an `ensures`-only atom exempts nothing is not built: the partner rule makes a `requires` atom necessary
    // a global that holds the cell only inside an array (`int g[4]`) is read as the leaf walker reads it
    rc("e1e2r_rej_global_array_holds_the_cell", "the body names or takes the address of the global `g`",
       [=](Kit& k)
       {
           Global* g {k.prog.add_global("g", k.prog.array_of(k.i32, 4U), k.at(), nullptr)};
           Function* f {restrict_fn(k, {})};
           f->body = k.blk({k.store(k.b.elem_addr(ga(k, g), N(k, "0"), k.at()), N(k, "2")), k.ret(N(k, "0"))});
           k.done(f);
       });
    // a structure global with an `int a[4]` member
    rc("e1e2r_rej_global_structure_with_an_array_member_holds_the_cell",
       "the body names or takes the address of the global `g`",
       [=](Kit& k)
       {
           StructType* A {k.strukt("A", {{"a", k.prog.array_of(k.i32, 4U)}, {"n", k.i64}})};
           Global* g {k.prog.add_global("g", k.st(A), k.at(), nullptr)};
           Function* f {restrict_fn(k, {})};
           f->body = k.blk({k.store(k.b.member_addr(ga(k, g), 1, k.at()), k.num(k.i64, "1")), k.ret(N(k, "0"))});
           k.done(f);
       });
    // a global that cannot hold the cell is not named by the rule (`long h`, held by a guarded atom, written under the guard, beside `int *restrict p`): accepted
    lc("e1e2r_unrelated_global_is_accepted", "a global of another cell type is no alias of *p",
       [=](Kit& k)
       {
           Global* h {k.prog.add_global("h", k.i64, k.at(), nullptr)};
           h->init = k.num(k.i64, "0");
           h->value = "0";
           Function* f {restrict_fn(k, {{AtomKind::Owns, ga(k, h)}}, true)};
           f->body = k.blk({k.store(k.pv(f, 0), N(k, "1")),
                            k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blkc({k.store(ga(k, h), k.num(k.i64, "2"))})),
                            k.ret(k.deref(k.pv(f, 0)))});
           k.done(f);
       });
}

// Audit round 1, issue 8: every way out of the scope of an array of structures checks and exhales exactly the (receiver, field) quantifiers its allocation gave, once each.  The
// emitted text is read: the allocation's `ensures forall ... acc(R.f)` lines are the set A (the allocated array's name written `r`), and every run of consecutive
// `assert forall ... acc(R.f)` lines and every run of consecutive `exhale forall ... acc(R.f)` lines of the method is exactly A.
std::function<std::string(const std::string&)> exits_pin_min(size_t min_exits)
{
    return [min_exits](const std::string& vpr) -> std::string
    {
        auto normalize = [](std::string t)
        {
            // the Viper local of the array (`v_v__1`, `v_w__3`) is `r`
            static const std::regex local {R"(v_[A-Za-z0-9_]+__[0-9]+)"};
            return std::regex_replace(t, local, "r");
        };
        static const std::regex quant {R"(forall k: Int :: \{ (.+?) \} .*==> acc\((.+)\.([vd]_[A-Za-z0-9_]+)\))"};
        std::vector<std::string> lines;
        std::istringstream in(vpr);
        for (std::string l; std::getline(in, l);)
        {
            lines.push_back(l);
        }
        std::multiset<std::string> alloc;
        bool in_alloc {false};
        std::vector<std::pair<std::string, std::multiset<std::string>>>
            runs; // (kind, the pairs of a run of consecutive lines)
        std::string last_kind;
        for (const std::string& l : lines)
        {
            if (l.rfind("method alloc_arr_struct", 0U) == 0U)
            {
                in_alloc = true;
                continue;
            }
            if (l.rfind("method ", 0U) == 0U)
            {
                in_alloc = false;
            }
            std::smatch m;
            std::string kind;
            if (in_alloc && l.find("ensures forall") != std::string::npos)
            {
                kind = "alloc";
            }
            else if (l.find("exhale forall") != std::string::npos)
            {
                kind = "exhale";
            }
            else if (l.find("assert forall") != std::string::npos)
            {
                kind = "marked";
            }
            if (!kind.empty() && std::regex_search(l, m, quant) && m[2].str().find("member(") != std::string::npos)
            {
                const std::string pair {normalize(m[2].str()) + "." + m[3].str()};
                if (kind == "alloc")
                {
                    alloc.insert(pair);
                    continue;
                }
                if (runs.empty() || last_kind != kind)
                {
                    runs.emplace_back(kind, std::multiset<std::string> {});
                }
                runs.back().second.insert(pair);
                last_kind = kind;
                continue;
            }
            last_kind.clear();
        }
        if (alloc.empty())
        {
            return "the allocation states no leaf quantifier";
        }
        // each (receiver, field) once in the allocation
        for (const std::string& a : alloc)
        {
            if (alloc.count(a) != 1U)
            {
                return "the allocation gives " + a + " more than once";
            }
        }
        size_t exits {0U};
        for (const auto& [kind, set] : runs)
        {
            if (set != alloc)
            {
                std::string got;
                for (const std::string& g : set)
                {
                    got += " " + g;
                }
                return "a run of `" + kind + "` rows of a scope exit differs from the allocation's quantifiers:" + got;
            }
            exits += kind == "exhale" ? 1U : 0U;
        }
        return exits >= min_exits
                   ? ""
                   : "expected at least " + std::to_string(min_exits) + " scope exits, found " + std::to_string(exits);
    };
}

// ---- decision 0064 items 3, 5 and 6 (Part B): an automatic local array of structures ----
void define_array_of_structures_lowering()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // struct P { int x; int y; }
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"x", k.i32}, {"y", k.i32}});
    };
    // struct P v[n];
    auto arr = [](Kit& k, Function* f, StructType* P, unsigned n, bool zero)
    {
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), n), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        return v;
    };
    auto elem = [](Kit& k, Var* v, const Expr* i)
    {
        return k.b.elem_addr(k.addr(v), i, k.at());
    };
    // v[i].m as a place and as a read
    auto place = [=](Kit& k, Var* v, const Expr* i, int m)
    {
        return k.b.member_addr(elem(k, v, i), m, k.at());
    };
    auto read = [=](Kit& k, Var* v, const Expr* i, int m)
    {
        return k.deref(place(k, v, i, m));
    };

    // int f(int i) requires(0 <= i <= hi) ensures(r == 7) { struct P v[3]; v[i].x = 3; v[i].y = 4; return v[i].x + v[i].y; }
    const auto elem_rw = [=](Kit& k, const char* hi, bool two) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"i", k.i32}})};
        k.req(f, k.range(k.F(f, 0), k.i32, "0", hi));
        Var* v {arr(k, f, P, 3U, false)};
        const Stmt* w1 {k.store(place(k, v, k.pv(f, 0), 0), N(k, "3"))};
        std::vector<const Stmt*> body {k.decl(v, nullptr), w1};
        if (two)
        {
            body.push_back(k.store(place(k, v, k.pv(f, 0), 1), N(k, "4")));
            body.push_back(k.ret(k.add(read(k, v, k.pv(f, 0), 0), read(k, v, k.pv(f, 0), 1))));
            k.ens(f, k.eq(k.R(f), N(k, "7")));
        }
        else
        {
            body.push_back(k.ret(N(k, "0")));
        }
        f->body = k.blkc(body);
        k.done(f);
        return w1->loc;
    };
    lc("e1e2b_elem_rw", "v[i].x and v[i].y written and read for a symbolic index of a local array of structures",
       [=](Kit& k)
       {
           elem_rw(k, "2", true);
       });
    lc("e1e2b_elem_index_high_fail", "v[i].x with i == 3: out-of-bounds at the access, the bound row before the cell's",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", elem_rw(k, "3", false));
       });
    // int f(void) { struct P v[3] = {0}; check(v[2].y == [0|1]); }
    const auto zero = [=](Kit& k, const char* want) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32)};
        Var* v {arr(k, f, P, 3U, true)};
        const Stmt* c {k.chk(k.eq(read(k, v, N(k, "2"), 1), N(k, want)))};
        f->body = k.blkc({k.decl(v, nullptr), c, k.ret(N(k, "0"))});
        k.done(f);
        return c->loc;
    };
    lc("e1e2b_zero_fill", "an array of structures with the zero fill: every leaf of every element is zero and set",
       [=](Kit& k)
       {
           zero(k, "0");
       });
    lc("e1e2b_zero_fill_wrong_fail", "the zero fill: the check that v[2].y is 1 fails",
       [=](Kit& k)
       {
           k.fail("check", zero(k, "1"));
       });
    const auto exits_pin = exits_pin_min(3U);
    // int f(int c) { struct P v[2]; if (c != 0) { return 1; } { struct P w[2]; w[0].x = 1; } return 0; }: the return in the `if`, the end of the inner block, the final return
    lc("e1e2b_scope_exits_give_what_the_allocation_gave",
       "every way out of an array of structures' scope exhales exactly the allocation's quantifiers",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32, {{"c", k.i32}})};
           Var* v {arr(k, f, P, 2U, false)};
           Var* w {arr(k, f, P, 2U, false)};
           const Stmt* inner {k.blkc({k.decl(w, nullptr), k.store(place(k, w, N(k, "0"), 0), N(k, "1"))})};
           f->body = k.blkc({k.decl(v, nullptr), k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({k.ret(N(k, "1"))})), inner,
                             k.ret(N(k, "0"))});
           k.done(f);
           k.text_checks.push_back(exits_pin);
       });
    // ---- sequencing (item 11): the leaves of the elements of a local array of structures are objects of their own ----
    // void f(int i, int j) requires(0 <= i <= 2 && 0 <= j <= 2) { struct P v[3]; (every leaf written); v[<li>].<lm> = v[<ri>].<rm>++; }
    const auto seq = [=](Kit& k, bool variable, int lm, int rm, bool same_index, const Expr** out_outer)
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", "2"), k.range(k.F(f, 1), k.i32, "0", "2")));
        Var* v {arr(k, f, P, 3U, true)};
        auto ix = [&](bool left)
        {
            return variable ? (left || same_index ? k.pv(f, 0) : k.pv(f, 1)) : N(k, left || same_index ? "0" : "1");
        };
        const Expr* lhs {place(k, v, ix(true), lm)};
        const Expr* rhs {place(k, v, ix(false), rm)};
        const Expr* inner {k.b.inc_dec_store_expr(rhs, true, true, k.at())};
        const Expr* outer {k.b.store_expr(lhs, inner, k.at())};
        f->body = k.blkc({k.decl(v, nullptr), k.es(outer)});
        k.done(f);
        *out_outer = outer;
    };
    lc("e1e2b_seq_distinct_elements", "v[0].x = v[1].x++: two elements are two objects: no unsequenced row fails",
       [=](Kit& k)
       {
           const Expr* o {nullptr};
           seq(k, false, 0, 0, false, &o);
       });
    lc("e1e2b_seq_two_leaves_of_one_element", "v[i].x = v[i].y++: two leaves of one element are two objects",
       [=](Kit& k)
       {
           const Expr* o {nullptr};
           seq(k, true, 0, 1, true, &o);
       });
    lc("e1e2b_seq_unknown_indexes_fail",
       "v[i].x = v[j].x++ with i and j unknown: the two elements may be one leaf: unsequenced at the later access",
       [=](Kit& k)
       {
           const Expr* o {nullptr};
           seq(k, true, 0, 0, false, &o);
           k.fail_only("unsequenced", o->loc, 1U, 0U);
       });
    rc("e1e2b_rej_seq_same_leaf", "unsequenced modification of",
       [=](Kit& k)
       {
           const Expr* o {nullptr};
           seq(k, false, 0, 0, true, &o);
       });
    // audit round 1, issue 9 and note 15: a structure store writes every leaf of its destination element: `v[0] = (struct P){v[0].x++, 0};`
    rc("e1e2b_rej_seq_struct_store_writes_a_leaf_of_its_destination", "unsequenced modification of",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 3U, true)};
           const Expr* inc {k.b.inc_dec_store_expr(place(k, v, N(k, "0"), 0), true, true, k.at())};
           const Expr* value {k.b.aggregate(k.st(P), {inc, N(k, "0")}, k.at())};
           f->body = k.blkc({k.decl(v, nullptr), k.store(elem(k, v, N(k, "0")), value)});
           k.done(f);
       });
    // void set(struct P *p) requires(owns(*p)) ensures(owns(*p) && p->x == 7) { p->x = 7; }   [keep: requires(owns(*p)) only, a callee that keeps the cells]
    // int f(int i) requires(0 <= i < 3) { struct P v[3]; set(&v[i]); check(v[i].x == 7)?; }
    const auto callee = [=](Kit& k, StructType* P, bool keep) -> Function*
    {
        Function* g {k.fn(keep ? "keep" : "set", k.vd, {{"p", k.ptr(k.st(P))}})};
        k.req(g, k.b.atom(AtomKind::Owns, k.F(g, 0), 0, k.at()));
        if (!keep)
        {
            k.ens(g, k.land(k.b.atom(AtomKind::Owns, k.F(g, 0), 0, k.at()), k.eq(k.leaf(k.F(g, 0), 0), N(k, "7"))));
            g->body = k.blkc({k.lstore(k.pv(g, 0), 0, N(k, "7"))});
        }
        else
        {
            g->body = k.blk({});
        }
        k.done(g);
        return g;
    };
    const auto addr_call = [=](Kit& k, bool keep) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {callee(k, P, keep)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}})};
        k.req(f, k.range(k.F(f, 0), k.i32, "0", "2"));
        Var* v {arr(k, f, P, 3U, false)};
        std::vector<const Stmt*> body {k.decl(v, nullptr), k.store(place(k, v, k.pv(f, 0), 0), N(k, "1")),
                                       k.store(place(k, v, k.pv(f, 0), 1), N(k, "2")),
                                       k.es(k.call(g, {elem(k, v, k.pv(f, 0))}))};
        if (!keep)
        {
            body.push_back(k.chk(k.eq(read(k, v, k.pv(f, 0), 0), N(k, "7"))));
        }
        const Stmt* bs {k.blkc(body)};
        f->body = bs;
        k.done(f);
        return bs->end_loc;
    };
    lc("e1e2b_address_to_callee",
       "&v[i] given to a callee that owns the element and gives it back: the element reads 7",
       [=](Kit& k)
       {
           addr_call(k, false);
       });
    lc("e1e2b_address_kept_fail",
       "&v[i] given to a callee that keeps the cells: lifetime at the first marked scope-exit row",
       [=](Kit& k)
       {
           k.fail_only("lifetime", addr_call(k, true), 5U, 0U);
       });
}

// ---- decision 0064 item 10 (Part B): loops over a local array of structures: every cell leaf of the elements is an array place of the loop ----
void define_array_of_structures_loops()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto L = [](Kit& k, const char* v)
    {
        return k.num(k.i64, v);
    };
    // struct P { int x; int y; }, struct Q { int x; long y; }
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"x", k.i32}, {"y", k.i32}});
    };
    auto mk_q = [](Kit& k)
    {
        return k.strukt("Q", {{"x", k.i32}, {"y", k.i64}});
    };
    auto arr = [](Kit& k, Function* f, StructType* P, unsigned n, bool zero)
    {
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), n), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        return v;
    };
    auto elem = [](Kit& k, Var* v, const Expr* i)
    {
        return k.b.elem_addr(k.addr(v), i, k.at());
    };
    auto place = [=](Kit& k, Var* v, const Expr* i, int m)
    {
        return k.b.member_addr(elem(k, v, i), m, k.at());
    };
    auto read = [=](Kit& k, Var* v, const Expr* i, int m)
    {
        return k.deref(place(k, v, i, m));
    };
    // v[idx].x = xs[idx]; v[idx].y = ys[idx] for the three elements (the leaves' initial values)
    auto init3 = [=](Kit& k, Var* v, const Type& yt, std::vector<const Stmt*>& out)
    {
        const char* xs[] {"1", "3", "5"};
        const char* ys[] {"2", "4", "6"};
        const char* idx[] {"0", "1", "2"};
        for (int e {0}; e < 3; ++e)
        {
            out.push_back(k.store(place(k, v, N(k, idx[e]), 0), N(k, xs[e])));
            out.push_back(k.store(place(k, v, N(k, idx[e]), 1), k.num(yt, ys[e])));
        }
    };
    // int f(void) ensures(0 <= r && r <= 15) { struct P v[3] = {{1,2},{3,4},{5,6}}; int s = 0, i = 0;
    //   while (i < 3) inv(0 <= i <= 3, 0 <= s, s <= 5 * i) dec(3 - i) { [v[i].x = 7;] s = s + v[i].x; i = i + 1; } return s; }
    const auto sum = [=](Kit& k, bool write7) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32)};
        k.ens(f, k.range(k.R(f), k.i32, "0", "15"));
        Var* v {arr(k, f, P, 3U, false)};
        Var* s {k.local(f, "s", k.i32)};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> top {k.decl(v, nullptr)};
        init3(k, v, k.i32, top);
        top.push_back(k.decl(s, N(k, "0")));
        top.push_back(k.decl(i, N(k, "0")));
        std::vector<const Stmt*> body;
        if (write7)
        {
            body.push_back(k.store(place(k, v, k.v(i), 0), N(k, "7")));
        }
        body.push_back(k.asg(s, k.add(k.v(s), read(k, v, k.v(i), 0))));
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Clause c1 {k.cl(k.range(k.v(i), k.i32, "0", "3"))};
        const Clause c2 {k.cl(k.ge_c(k.v(s), k.i32, "0"))};
        const Clause c3 {k.cl(k.le(k.v(s), k.mul(N(k, "5"), k.v(i))))};
        top.push_back(k.whl(k.lt(k.v(i), N(k, "3")), {c1, c2, c3}, {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(body)));
        top.push_back(k.ret(k.v(s)));
        f->body = k.blkc(top);
        k.done(f);
        return c3.loc;
    };
    lc("e1e2b_loop_sum",
       "a loop reads v[i].x and relies on the values of x kept (no write reaches int cells): s <= 5 * i holds",
       [=](Kit& k)
       {
           sum(k, false);
       });
    lc("e1e2b_loop_sum_written_fail",
       "the loop writes v[i].x = 7: a store to a member reaches every int place, the values of x are not kept, s <= 5 * i is not preserved (one error at the clause)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", sum(k, true));
       });
    // int f(int n) requires(0 <= n <= 3) ensures(r == n) { struct Q v[3] = {...}; int i = 0;
    //   while (i < n) inv(0 <= i <= n) dec(n - i) { v[i].x = i; [v[i].y = 5;] i = i + 1; } check(v[1].y == 4 && v[2].y == 6); return i; }
    const auto fill = [=](Kit& k, bool write_y) -> SrcLoc
    {
        StructType* Q {mk_q(k)};
        Function* f {k.fn("f", k.i32, {{"n", k.i32}})};
        k.req(f, k.range(k.F(f, 0), k.i32, "0", "3"));
        k.ens(f, k.eq(k.R(f), k.F(f, 0)));
        Var* v {arr(k, f, Q, 3U, false)};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> top {k.decl(v, nullptr)};
        init3(k, v, k.i64, top);
        top.push_back(k.decl(i, N(k, "0")));
        std::vector<const Stmt*> body {k.store(place(k, v, k.v(i), 0), k.v(i))};
        if (write_y)
        {
            body.push_back(k.store(place(k, v, k.v(i), 1), L(k, "5")));
        }
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Clause c1 {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 0))))};
        top.push_back(k.whl(k.lt(k.v(i), k.pv(f, 0)), {c1}, {k.cl(k.sub(k.pv(f, 0), k.v(i)))}, k.blkc(body)));
        const Stmt* c {
            k.chk(k.land(k.eq(read(k, v, N(k, "1"), 1), L(k, "4")), k.eq(read(k, v, N(k, "2"), 1), L(k, "6"))))};
        top.push_back(c);
        top.push_back(k.ret(k.v(i)));
        f->body = k.blkc(top);
        k.done(f);
        return c->loc;
    };
    lc("e1e2b_loop_fill",
       "the loop writes the int member only: the long member's values are kept, the check after the loop holds",
       [=](Kit& k)
       {
           fill(k, false);
       });
    lc("e1e2b_loop_fill_other_leaf_fail",
       "the loop also writes the long member: its values are not kept, the check after the loop fails",
       [=](Kit& k)
       {
           k.fail("check", fill(k, true));
       });
    // int f(void) ensures(r == 3) { struct P v[3] = {{0,2},{0,4},{0,6}}; int i = 1; while (i < 3) inv(1 <= i <= 3) dec(3 - i) { v[i].x = 5; i = i + 1; } check(v[0].x == 0); return i; }
    lc("e1e2b_loop_keep_false_alarm",
       "a store to v[i].x reaches every int place: v[0].x == 0 after the loop is true of the program but not kept (a pinned false alarm, one error at the check)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32)};
           k.ens(f, k.eq(k.R(f), N(k, "3")));
           Var* v {arr(k, f, P, 3U, false)};
           Var* i {k.local(f, "i", k.i32)};
           std::vector<const Stmt*> top {k.decl(v, nullptr)};
           for (int e {0}; e < 3; ++e)
           {
               top.push_back(k.store(place(k, v, N(k, std::to_string(e).c_str()), 0), N(k, "0")));
               top.push_back(
                   k.store(place(k, v, N(k, std::to_string(e).c_str()), 1), N(k, std::to_string(2 * e + 2).c_str())));
           }
           top.push_back(k.decl(i, N(k, "1")));
           const Clause c1 {k.cl(k.range(k.v(i), k.i32, "1", "3"))};
           top.push_back(
               k.whl(k.lt(k.v(i), N(k, "3")), {c1}, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                     k.blkc({k.store(place(k, v, k.v(i), 0), N(k, "5")), k.asg(i, k.add(k.v(i), N(k, "1")))})));
           const Stmt* c {k.chk(k.eq(read(k, v, N(k, "0"), 0), N(k, "0")))};
           top.push_back(c);
           top.push_back(k.ret(k.v(i)));
           f->body = k.blkc(top);
           k.done(f);
           k.fail("check", c->loc);
       });
    // invariant reads of a member of an element of the named local array (decision 0064 item 10): v[1].y == 4 (long, not reached), and `i == 3 || v[i].y >= 0`
    // int f(void) { struct Q v[3] = {...}; int i = 0; while (i < 3) inv(0 <= i <= 3, v[1].y == 4, i == 3 || v[i].y >= 0) dec(3 - i) { v[i].x = i; i = i + 1; } return i; }
    // `mode`: 0 the reads above; 1 `v[i].y >= 0` alone (the index reaches 3: out-of-bounds); 2 `v[0].y >= 0` over an array no element of which was written (uninitialized)
    const auto inv_reads = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* Q {mk_q(k)};
        Function* f {k.fn("f", k.i32)};
        Var* v {arr(k, f, Q, 3U, false)};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> top {k.decl(v, nullptr)};
        if (mode != 2)
        {
            init3(k, v, k.i64, top);
        }
        top.push_back(k.decl(i, N(k, "0")));
        const Clause c1 {k.cl(k.range(k.v(i), k.i32, "0", "3"))};
        std::vector<Clause> inv {c1};
        const Expr* rd {nullptr};
        Clause probe {k.cl(k.eq(read(k, v, N(k, "1"), 1), L(k, "4")))};
        if (mode == 0)
        {
            inv.push_back(probe);
            probe = k.cl(k.lor(k.eq(k.v(i), N(k, "3")), k.ge_c(read(k, v, k.v(i), 1), k.i64, "0")));
        }
        else
        {
            // the obligation is reported at the read itself
            rd = mode == 1 ? read(k, v, k.v(i), 1) : read(k, v, N(k, "0"), 1);
            probe = k.cl(k.ge_c(rd, k.i64, "0"));
        }
        inv.push_back(probe);
        top.push_back(k.whl(k.lt(k.v(i), N(k, "3")), inv, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                            k.blkc({k.store(place(k, v, k.v(i), 0), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))})));
        top.push_back(k.ret(k.v(i)));
        f->body = k.blkc(top);
        k.done(f);
        return rd != nullptr ? rd->loc : probe.loc;
    };
    lc("e1e2b_loop_inv_reads_members",
       "invariants that read v[1].y and v[i].y of the named local array: the loop carries the leaf, the reads are framed and in bounds",
       [=](Kit& k)
       {
           inv_reads(k, 0);
       });
    lc("e1e2b_loop_inv_reads_index_high_fail",
       "the invariant v[i].y >= 0 reads one element too far when i reaches 3: out-of-bounds at the read, at the back edge (the entry holds)",
       [=](Kit& k)
       {
           k.fail_only("out-of-bounds", inv_reads(k, 1), 2U, 1U);
       });
    lc("e1e2b_loop_inv_reads_unwritten_fail",
       "the invariant reads v[0].y of an array no leaf of which was written: uninitialized at the clause",
       [=](Kit& k)
       {
           k.fail_only("uninitialized", inv_reads(k, 2), 2U, 0U);
       });
    // ---- a read through a pointer into the array, in an invariant: framed by the leaf's place (array_place_hit recovers the element from the leaf cell's block) ----
    // long f(void) ensures(r == 4) { struct Q v[3] = {...}; long *q = &v[1].y; int i = 0; while (i < 3) inv(0 <= i <= 3, *q == 4) dec(3 - i) { v[i].x = i; i = i + 1; } return *q; }
    // `mode` 1: the pointer is `&v[1].x`, an int cell the loop writes, and the invariant reads it as int: not kept
    const auto through_pointer = [=](Kit& k, bool wrong) -> SrcLoc
    {
        StructType* Q {mk_q(k)};
        Function* f {k.fn("f", wrong ? k.i32 : k.i64)};
        if (!wrong)
        {
            k.ens(f, k.eq(k.R(f), L(k, "4")));
        }
        Var* v {arr(k, f, Q, 3U, false)};
        Var* i {k.local(f, "i", k.i32)};
        Var* q {k.local(f, "q", k.ptr(wrong ? k.i32 : k.i64))};
        std::vector<const Stmt*> top {k.decl(v, nullptr)};
        init3(k, v, k.i64, top);
        top.push_back(k.decl(q, place(k, v, N(k, "1"), wrong ? 0 : 1)));
        top.push_back(k.decl(i, N(k, "0")));
        const Clause c1 {k.cl(k.range(k.v(i), k.i32, "0", "3"))};
        const Clause c2 {k.cl(k.eq(k.deref(k.v(q)), wrong ? N(k, "3") : L(k, "4")))};
        top.push_back(k.whl(k.lt(k.v(i), N(k, "3")), {c1, c2}, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                            k.blkc({k.store(place(k, v, k.v(i), 0), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))})));
        top.push_back(k.ret(k.deref(k.v(q))));
        f->body = k.blkc(top);
        k.done(f);
        return c2.loc;
    };
    lc("e1e2b_loop_inv_reads_through_pointer",
       "*q in the invariant, q = &v[1].y: the read is framed by the place of the leaf y (the long member the loop does not write), the value is kept",
       [=](Kit& k)
       {
           through_pointer(k, false);
       });
    lc("e1e2b_loop_inv_reads_through_pointer_written_fail",
       "*q in the invariant, q = &v[1].x, an int cell the loop writes (a store to a member reaches every int place): *q == 3 is not preserved (one error at the clause)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", through_pointer(k, true));
       });
    // ---- a pointer-typed leaf of an element: the leaf place holds pointers (its range row is the pointer's well-formedness) ----
    // long f(void) ensures(r == 3) { long x = 3; struct R v[2] (R { long *p; int n; }); v[0].p = &x; v[0].n = 0; v[1].p = &x; v[1].n = 0; int i = 0; while (i < 2) inv(0 <= i <= 2) dec(2 - i) { v[i].n = i; i = i + 1; } return *v[0].p; }
    lc("e1e2b_loop_pointer_leaf",
       "a loop over an array of structures with a pointer member: the pointer leaf's place is carried, its values are kept, and so is the long cell it points to (the loop writes the int member only)",
       [=](Kit& k)
       {
           StructType* R {k.strukt("R", {{"p", k.ptr(k.i64)}, {"n", k.i32}})};
           Function* f {k.fn("f", k.i64)};
           k.ens(f, k.eq(k.R(f), L(k, "3")));
           Var* x {k.cell(f, "x", k.i64)};
           Var* v {f->new_local("v", k.prog.array_of(k.st(R), 2U), k.at())};
           f->mark_address_taken(v);
           Var* i {k.local(f, "i", k.i32)};
           auto at_elem = [&](const Expr* idx, int m)
           {
               return k.b.member_addr(k.b.elem_addr(k.addr(v), idx, k.at()), m, k.at());
           };
           std::vector<const Stmt*> top {k.decl(x, L(k, "3")), k.decl(v, nullptr)};
           for (const char* e : {"0", "1"})
           {
               top.push_back(k.store(at_elem(N(k, e), 0), k.addr(x)));
               top.push_back(k.store(at_elem(N(k, e), 1), N(k, "0")));
           }
           top.push_back(k.decl(i, N(k, "0")));
           const Clause c1 {k.cl(k.range(k.v(i), k.i32, "0", "2"))};
           top.push_back(k.whl(k.lt(k.v(i), N(k, "2")), {c1}, {k.cl(k.sub(N(k, "2"), k.v(i)))},
                               k.blkc({k.store(at_elem(k.v(i), 1), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))})));
           top.push_back(k.ret(k.deref(k.deref(at_elem(N(k, "0"), 0)))));
           f->body = k.blkc(top);
           k.done(f);
       });
    // ---- a parameter's place beside the leaf places: its guard and case splits read the leaf's receiver (array_place_hit), never the array's block ----
    // long f(long *p) requires(owns(*p) && *p == 5) ensures(owns(*p) && r == 5) { struct Q v[3]; int i = 0; while (i < 3) inv(0 <= i <= 3) dec(3 - i) { v[i].x = i; i = i + 1; } return *p; }
    const auto param_place = [=](Kit& k, bool int_place) -> SrcLoc
    {
        StructType* Q {mk_q(k)};
        const Type pt {int_place ? k.i32 : k.i64};
        const auto num = [&](const char* v) -> const Expr*
        {
            return k.num(pt, v);
        };
        Function* f {k.fn("f", pt, {{"p", k.ptr(pt)}})};
        k.req(f, k.land(k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()), k.eq(k.deref(k.F(f, 0)), num("5"))));
        const Expr* post {k.eq(k.R(f), num("5"))};
        k.ens(f, k.land(k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()), post));
        Var* v {arr(k, f, Q, 3U, false)};
        Var* i {k.local(f, "i", k.i32)};
        const Clause c1 {k.cl(k.range(k.v(i), k.i32, "0", "3"))};
        const Stmt* ret {k.ret(k.deref(k.pv(f, 0)))};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(i, N(k, "0")),
                          k.whl(k.lt(k.v(i), N(k, "3")), {c1}, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                                k.blkc({k.store(place(k, v, k.v(i), 0), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))})),
                          ret});
        k.done(f);
        return ret->loc;
    };
    lc("e1e2b_loop_param_place_other_field",
       "a long parameter place beside the int leaf places of v: the loop writes int cells only, *p == 5 is kept across it",
       [=](Kit& k)
       {
           param_place(k, false);
       });
    lc("e1e2b_loop_param_place_same_field_fail",
       "an int parameter place: a store to v[i].x reaches every int place (0043 item 11), *p == 5 is not kept: the postcondition fails at the return (a pinned false alarm)",
       [=](Kit& k)
       {
           k.fail("postcondition", param_place(k, true));
       });
    // ---- a nested member is a leaf of its own: struct T { int a; long b; }; struct S { int x; struct T t; } ----
    // int f(void) { struct S v[3] = {...}; int i = 0; while (i < 3) inv(0 <= i <= 3, v[1].t.b == 4) dec(3 - i) { v[i].t.a = i; i = i + 1; } check(v[2].t.b == 6); return i; }
    lc("e1e2b_loop_nested_leaf",
       "the leaf path t.b has its own place: its values are kept while the loop writes t.a, and v[1].t.b is readable in the invariant",
       [=](Kit& k)
       {
           StructType* T {k.strukt("T", {{"a", k.i32}, {"b", k.i64}})};
           StructType* S {k.strukt("S", {{"x", k.i32}, {"t", k.st(T)}})};
           Function* f {k.fn("f", k.i32)};
           Var* v {f->new_local("v", k.prog.array_of(k.st(S), 3U), k.at())};
           f->mark_address_taken(v);
           Var* i {k.local(f, "i", k.i32)};
           auto elem_at = [&](const Expr* idx)
           {
               return k.b.elem_addr(k.addr(v), idx, k.at());
           };
           auto leaf_t = [&](const Expr* idx, int m)
           {
               return k.b.member_addr(k.b.member_addr(elem_at(idx), 1, k.at()), m, k.at());
           };
           std::vector<const Stmt*> top {k.decl(v, nullptr)};
           const char* idx[] {"0", "1", "2"};
           const char* bs[] {"2", "4", "6"};
           for (int e {0}; e < 3; ++e)
           {
               top.push_back(k.store(k.b.member_addr(elem_at(N(k, idx[e])), 0, k.at()), N(k, "1")));
               top.push_back(k.store(leaf_t(N(k, idx[e]), 0), N(k, "1")));
               top.push_back(k.store(leaf_t(N(k, idx[e]), 1), L(k, bs[e])));
           }
           // q = &v[1].t.b: the invariant's read through it recovers the element from the leaf cell by TWO `parent_of(block_of(.))` steps (the leaf path is {t, b})
           Var* q {k.local(f, "q", k.ptr(k.i64))};
           top.push_back(k.decl(q, leaf_t(N(k, "1"), 1)));
           top.push_back(k.decl(i, N(k, "0")));
           const Clause c1 {k.cl(k.range(k.v(i), k.i32, "0", "3"))};
           const Clause c2 {
               k.cl(k.land(k.eq(k.deref(leaf_t(N(k, "1"), 1)), L(k, "4")), k.eq(k.deref(k.v(q)), L(k, "4"))))};
           top.push_back(k.whl(k.lt(k.v(i), N(k, "3")), {c1, c2}, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                               k.blkc({k.store(leaf_t(k.v(i), 0), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))})));
           top.push_back(k.chk(k.eq(k.deref(leaf_t(N(k, "2"), 1)), L(k, "6"))));
           top.push_back(k.ret(k.v(i)));
           f->body = k.blkc(top);
           k.done(f);
       });
    // ---- what check_program keeps refusing (Part C): the element of a pointer in a loop invariant or measure ----
    rc("e1e2b_rej_loop_invariant_reads_a_pointer_element",
       "the element of a pointer to a structure in a loop invariant or measure",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           Var* i {k.local(f, "i", k.i32)};
           const Expr* rd {k.deref(k.b.member_addr(k.b.elem_addr(k.pv(f, 0), N(k, "0"), k.at()), 0, k.at()))};
           const Clause c1 {k.cl(k.range(k.v(i), k.i32, "0", "3"))};
           const Clause c2 {k.cl(k.ge_c(rd, k.i32, "0"))};
           f->body =
               k.blkc({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "3")), {c1, c2}, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                                                   k.blkc({k.asg(i, k.add(k.v(i), N(k, "1")))}))});
           k.done(f);
       });
    rc("e1e2b_rej_loop_measure_reads_a_pointer_element",
       "the element of a pointer to a structure in a loop invariant or measure",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           Var* i {k.local(f, "i", k.i32)};
           const Expr* rd {k.deref(k.b.member_addr(k.b.elem_addr(k.pv(f, 0), N(k, "0"), k.at()), 0, k.at()))};
           const Clause c1 {k.cl(k.range(k.v(i), k.i32, "0", "3"))};
           f->body = k.blkc(
               {k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "3")), {c1}, {k.cl(k.add(k.sub(N(k, "3"), k.v(i)), rd))},
                                            k.blkc({k.asg(i, k.add(k.v(i), N(k, "1")))}))});
           k.done(f);
       });
}

// ---- decision 0064 items 5, 7, 11 and 12 (Part B): call pairings, pointer bases, the decay, one past the end, jumps, element copies and sequencing ----
void define_array_of_structures_pairs_and_copies()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"x", k.i32}, {"y", k.i32}});
    };
    auto arr = [](Kit& k, Function* f, StructType* P, unsigned n, bool zero)
    {
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), n), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        return v;
    };
    auto elem = [](Kit& k, Var* v, const Expr* i)
    {
        return k.b.elem_addr(k.addr(v), i, k.at());
    };
    auto place = [=](Kit& k, Var* v, const Expr* i, int m)
    {
        return k.b.member_addr(elem(k, v, i), m, k.at());
    };
    auto read = [=](Kit& k, Var* v, const Expr* i, int m)
    {
        return k.deref(place(k, v, i, m));
    };
    // `p[i].m` over the pointer expression `base`
    auto pplace = [](Kit& k, const Expr* base, const Expr* i, int m)
    {
        return k.b.member_addr(k.b.elem_addr(base, i, k.at()), m, k.at());
    };
    auto owns = [](Kit& k, Function* f, int param)
    {
        return k.b.atom(AtomKind::Owns, k.F(f, param), param, k.at());
    };

    // ---- a structure atom against a cell atom whose arguments point into one local array ----
    // void g(struct P *p, int *q) requires(owns(*p) && owns(*q)) ensures(owns(*p) && owns(*q)) {}
    // void f(void) { struct P v[3]; g(&v[0], &v[<second>].x); }
    const auto pairing = [=](Kit& k, const char* second) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(P))}, {"q", k.ptr(k.i32)}})};
        k.req(g, k.land(owns(k, g, 0), owns(k, g, 1)));
        k.ens(g, k.land(owns(k, g, 0), owns(k, g, 1)));
        g->body = k.blk({});
        k.done(g);
        Function* f {k.fn("f", k.vd, {{"c", k.i32}})};
        Var* v {arr(k, f, P, 3U, false)};
        const Expr* first {elem(k, v, N(k, second[0] == 'a' ? "0" : "1"))};
        const Expr* c {k.call(g, {first, place(k, v, N(k, "1"), 0)})};
        // `owns` needs the cells initialized: both leaves of the two elements the call touches
        std::vector<const Stmt*> body {k.decl(v, nullptr)};
        for (const char* e : {"0", "1"})
        {
            body.push_back(k.store(place(k, v, N(k, e), 0), N(k, "1")));
            body.push_back(k.store(place(k, v, N(k, e), 1), N(k, "2")));
        }
        body.push_back(k.es(c));
        f->body = k.blkc(body);
        k.done(f);
        return c->loc;
    };
    lc("e1e2b_pairing_distinct_elements",
       "g(&v[0], &v[1].x) against owns(*p) && owns(*q): the cell is in another element: no row fails",
       [=](Kit& k)
       {
           pairing(k, "a");
       });
    lc("e1e2b_pairing_same_element_fail",
       "g(&v[1], &v[1].x): the cell is a leaf of the structure atom's element: one permission row at the call",
       [=](Kit& k)
       {
           k.fail_only("permission", pairing(k, "b"), 2U, 0U, "twice, but one of the two places");
       });

    // ---- p[i].x on a pointer parameter: the null and liveness rows once, then the bound, then the cell's ----
    // int f(struct P *p) requires(owns(*p)) ensures(owns(*p) && r == 7) { p[<i>].x = 7; return p[<i>].x; }
    const auto pointer_base = [=](Kit& k, const char* idx, bool read_back, bool owned = true) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}})};
        if (owned)
        {
            k.req(f, owns(k, f, 0));
        }
        if (read_back)
        {
            k.ens(f, k.land(owns(k, f, 0), k.eq(k.R(f), N(k, "7"))));
        }
        else if (owned)
        {
            k.ens(f, owns(k, f, 0));
        }
        const Expr* w {pplace(k, k.pv(f, 0), N(k, idx), 0)};
        const Stmt* st {k.store(w, N(k, "7"))};
        const Expr* rd {read_back ? k.deref(pplace(k, k.pv(f, 0), N(k, idx), 0)) : nullptr};
        f->body = read_back ? k.blkc({st, k.ret(rd)}) : k.blkc({st, k.ret(N(k, "0"))});
        k.done(f);
        // the rows of one access, in source order (item 5): the pointer's null and liveness rows ONCE (the member step adds none), then the bound; each access has its own
        k.rows("null-dereference", st->loc, 1U);
        k.rows("lifetime", st->loc, 1U);
        k.rows("out-of-bounds", st->loc, 1U);
        if (rd != nullptr)
        {
            k.rows("null-dereference", rd->loc, 1U);
            k.rows("lifetime", rd->loc, 1U);
            k.rows("out-of-bounds", rd->loc, 1U);
        }
        return st->loc;
    };
    lc("e1e2b_pointer_base_first", "p[0].x written and read through the pointer parameter that owns *p",
       [=](Kit& k)
       {
           pointer_base(k, "0", true);
       });
    lc("e1e2b_pointer_base_second_fail",
       "p[1].x: the bound row fails first (out-of-bounds); the cell is held by no atom, so the write row fails too (permission), both at the store",
       [=](Kit& k)
       {
           // Silicon assumes the failed bound and goes on: the cell p[1].x is held by no atom, so its write row fails too (the same line, as for a scalar p[1])
           const SrcLoc at {pointer_base(k, "1", false)};
           k.fail("out-of-bounds", at);
           k.fail("permission", at);
       });
    // int f(struct P *p) { p[0].x = 7; return 0; }  with no atom: the rows fail in order, the null row first (Silicon assumes each failed row and goes on)
    lc("e1e2b_pointer_base_unowned_fail",
       "p[0].x with no atom on p: the null-dereference and lifetime rows fail at the store, each once, in that order (Silicon assumes the failed liveness permission, which the heap lacks: the later rows are on an infeasible path)",
       [=](Kit& k)
       {
           const SrcLoc at {pointer_base(k, "0", false, false)};
           k.fail("null-dereference", at);
           k.fail("lifetime", at);
       });
    // void f(void) { struct P v[3]; struct P *q = &v[0]; q[1].x = 3; check(v[1].x == 3); }   (the decay is `&v[0]`)
    const auto decay = [=](Kit& k, const char* idx) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd)};
        Var* v {arr(k, f, P, 3U, false)};
        Var* q {k.local(f, "q", k.ptr(k.st(P)))};
        const Stmt* st {k.store(pplace(k, k.v(q), N(k, idx), 0), N(k, "3"))};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(q, elem(k, v, N(k, "0"))), st,
                          k.chk(k.eq(read(k, v, N(k, "1"), 0), N(k, "3")))});
        k.done(f);
        return st->loc;
    };
    lc("e1e2b_decay_pointer_base",
       "struct P *q = v; q[1].x = 3 writes v[1].x: the pointer into the array and the array's own read agree",
       [=](Kit& k)
       {
           decay(k, "1");
       });
    lc("e1e2b_decay_pointer_base_high_fail", "q[3].x: the bound is the array's length: out-of-bounds",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", decay(k, "3"));
       });
    // ---- one past the end: `&v[3]` as a value is allowed, `&v[3].x` forms a member of an element that does not exist ----
    lc("e1e2b_address_one_past_value", "struct P *e = &v[3];: one past the end, as a value (decision 0059 item 10)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 3U, false)};
           Var* e {k.local(f, "e", k.ptr(k.st(P)))};
           f->body = k.blkc({k.decl(v, nullptr), k.decl(e, elem(k, v, N(k, "3")))});
           k.done(f);
       });
    lc("e1e2b_address_one_past_member_fail",
       "int *z = &v[3].x;: the member of an element outside v: out-of-bounds at the address",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 3U, false)};
           Var* z {k.local(f, "z", k.ptr(k.i32))};
           const Expr* a {place(k, v, N(k, "3"), 0)};
           f->body = k.blkc({k.decl(v, nullptr), k.decl(z, a)});
           k.done(f);
           k.fail("out-of-bounds", a->loc);
       });
    // ---- a jump out of the array's scope exhales what the allocation gave (audit issue 8) ----
    // int f(int c) { { struct P v[2]; v[0].x = 1; if (c != 0) goto out; v[1].x = 2; } out: return 0; }
    lc("e1e2b_scope_exit_by_goto",
       "a goto out of the block that holds an array of structures: its exit exhales the allocation's quantifiers, as the block's end does",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32, {{"c", k.i32}})};
           Var* v {arr(k, f, P, 2U, false)};
           const int L {k.jid()};
           const Stmt* inner {k.blkc({k.decl(v, nullptr), k.store(place(k, v, N(k, "0"), 0), N(k, "1")),
                                      k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({k.go(L)})),
                                      k.store(place(k, v, N(k, "1"), 0), N(k, "2"))})};
           f->body = k.blkc({inner, k.lab(L), k.ret(N(k, "0"))});
           k.done(f);
           k.text_checks.push_back(exits_pin_min(2U));
       });
    // ---- element copies as the encoder emits them: leaf stores whose two sides are `MemberAddr(ElemAddr(..))` (edg-backend report 4, copy_struct) ----
    // void f(int i) requires(0 <= i <= hi) { struct P v[3] (leaf `skip` unwritten); struct P t; t.x = v[i].x; t.y = v[i].y; check(t.x == v[i].x); }
    const auto copy_out = [=](Kit& k, const char* hi, int skip, bool by_value) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}})};
        k.req(f, k.range(k.F(f, 0), k.i32, "0", hi));
        Var* v {arr(k, f, P, 3U, false)};
        std::vector<const Stmt*> body {k.decl(v, nullptr)};
        for (int e {0}; e < 3; ++e)
        {
            for (int m {0}; m < 2; ++m)
            {
                if (m != skip)
                {
                    body.push_back(k.store(place(k, v, N(k, std::to_string(e).c_str()), m), N(k, "4")));
                }
            }
        }
        SrcLoc where;
        if (by_value)
        {
            // int h(struct P q) { return q.x; }   f: h(v[i]) as the Aggregate of the element's leaf reads
            Function* h {k.fn("h", k.i32, {{"q", k.st(P)}})};
            h->mark_address_taken(h->param_var(0));
            h->body = k.blkc({k.ret(k.leaf(k.addr(h->param_var(0)), 0))});
            k.done(h);
            const Expr* first {read(k, v, k.pv(f, 0), 0)};
            const Expr* second {read(k, v, k.pv(f, 0), 1)};
            where = skip == 1 ? second->loc : first->loc;
            body.push_back(k.es(k.call(h, {k.b.aggregate(k.st(P), {first, second}, k.at())})));
        }
        else
        {
            Var* t {k.scell(f, "t", P)};
            body.push_back(k.decl(t, nullptr));
            const Expr* first {read(k, v, k.pv(f, 0), 0)};
            const Expr* second {read(k, v, k.pv(f, 0), 1)};
            where = skip == 1 ? second->loc : first->loc;
            body.push_back(k.lstore(k.addr(t), 0, first));
            body.push_back(k.lstore(k.addr(t), 1, second));
            body.push_back(k.chk(k.eq(k.leaf(k.addr(t), 0), read(k, v, k.pv(f, 0), 0))));
        }
        f->body = k.blkc(body);
        k.done(f);
        return where;
    };
    lc("e1e2b_copy_out",
       "t = v[i] as leaf stores: every leaf of the element is read inside the array and copied, t.x == v[i].x after",
       [=](Kit& k)
       {
           copy_out(k, "2", -1, false);
       });
    lc("e1e2b_copy_out_index_high_fail",
       "t = v[i] with i == 3: the first leaf read is out-of-bounds, before its cell's rows",
       [=](Kit& k)
       {
           k.fail_only("out-of-bounds", copy_out(k, "3", -1, false), 1U, 0U);
       });
    lc("e1e2b_copy_out_unwritten_fail",
       "t = v[i] where v[i].y has no value: uninitialized at the read of the source leaf",
       [=](Kit& k)
       {
           k.fail("uninitialized", copy_out(k, "2", 1, false));
       });
    lc("e1e2b_by_value_argument",
       "h(v[i]): the element is passed as the Aggregate of its leaves, each read inside the array and initialized",
       [=](Kit& k)
       {
           copy_out(k, "2", -1, true);
       });
    lc("e1e2b_by_value_argument_unwritten_fail", "h(v[i]) where v[i].y has no value: uninitialized at that leaf's read",
       [=](Kit& k)
       {
           k.fail("uninitialized", copy_out(k, "2", 1, true));
       });
    // void f(int i) requires(0 <= i <= 2) { struct P t = {1, 2}; struct P v[3]; v[i].x = t.x; v[i].y = t.y; check(v[i].y == t.y); }   (`v[i] = t`: the destination is the element's leaves)
    const auto copy_in = [=](Kit& k, bool second_written) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}})};
        k.req(f, k.range(k.F(f, 0), k.i32, "0", "2"));
        Var* v {arr(k, f, P, 3U, false)};
        Var* t {k.scell(f, "t", P)};
        std::vector<const Stmt*> body {k.decl(v, nullptr), k.decl(t, nullptr), k.lstore(k.addr(t), 0, N(k, "1"))};
        if (second_written)
        {
            body.push_back(k.lstore(k.addr(t), 1, N(k, "2")));
        }
        const Expr* src {k.leaf(k.addr(t), 1)};
        body.push_back(k.store(place(k, v, k.pv(f, 0), 0), k.leaf(k.addr(t), 0)));
        body.push_back(k.store(place(k, v, k.pv(f, 0), 1), src));
        body.push_back(k.chk(k.eq(read(k, v, k.pv(f, 0), 1), k.leaf(k.addr(t), 1))));
        f->body = k.blkc(body);
        k.done(f);
        return src->loc;
    };
    lc("e1e2b_copy_in", "v[i] = t as leaf stores into the element: v[i].y == t.y after",
       [=](Kit& k)
       {
           copy_in(k, true);
       });
    lc("e1e2b_copy_in_unwritten_fail", "v[i] = t where t.y has no value: uninitialized at the source leaf",
       [=](Kit& k)
       {
           k.fail("uninitialized", copy_in(k, false));
       });
    // void f(int i, int j) requires(0 <= i, j <= 2) { struct P v[3] (x of every element written, y when `y_written`); v[i].x = v[j].x; v[i].y = v[j].y; check(v[i].x == v[j].x); }
    const auto copy_between = [=](Kit& k, bool y_written) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", "2"), k.range(k.F(f, 1), k.i32, "0", "2")));
        Var* v {arr(k, f, P, 3U, false)};
        std::vector<const Stmt*> body {k.decl(v, nullptr)};
        for (int e {0}; e < 3; ++e)
        {
            body.push_back(k.store(place(k, v, N(k, std::to_string(e).c_str()), 0), N(k, "1")));
            if (y_written)
            {
                body.push_back(k.store(place(k, v, N(k, std::to_string(e).c_str()), 1), N(k, "2")));
            }
        }
        const Expr* sx {read(k, v, k.pv(f, 1), 0)};
        const Expr* sy {read(k, v, k.pv(f, 1), 1)};
        body.push_back(k.store(place(k, v, k.pv(f, 0), 0), sx));
        body.push_back(k.store(place(k, v, k.pv(f, 0), 1), sy));
        body.push_back(k.chk(k.eq(read(k, v, k.pv(f, 0), 0), read(k, v, k.pv(f, 1), 0))));
        f->body = k.blkc(body);
        k.done(f);
        return sy->loc;
    };
    lc("e1e2b_copy_between_elements",
       "v[i] = v[j] with i == j possible: leaf stores with both sides inside the array, v[i].x == v[j].x after",
       [=](Kit& k)
       {
           copy_between(k, true);
       });
    lc("e1e2b_copy_between_elements_unwritten_fail",
       "v[i] = v[j] where no element's y has a value: uninitialized at the source leaf read",
       [=](Kit& k)
       {
           k.fail("uninitialized", copy_between(k, false));
       });
    // ---- whole-element sequencing (item 11): int g(struct P p, int n); g(v[<a>], v[<b>].x++) ----
    const auto whole = [=](Kit& k, bool variable, const char* second, const Expr** inc_out)
    {
        StructType* P {mk_p(k)};
        Function* g {k.fn("g", k.vd, {{"p", k.st(P)}, {"n", k.i32}})};
        g->mark_address_taken(g->param_var(0));
        g->body = k.blk({});
        k.done(g);
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", "2"), k.range(k.F(f, 1), k.i32, "0", "2")));
        Var* v {arr(k, f, P, 3U, true)};
        const Expr* ia {variable ? k.pv(f, 0) : N(k, "0")};
        const Expr* ib {variable ? k.pv(f, 1) : N(k, second)};
        const Expr* agg {k.b.aggregate(k.st(P), {read(k, v, ia, 0), read(k, v, ia, 1)}, k.at())};
        const Expr* inc {k.b.inc_dec_store_expr(place(k, v, ib, 0), true, true, k.at())};
        f->body = k.blkc({k.decl(v, nullptr), k.es(k.call(g, {agg, inc}))});
        k.done(f);
        *inc_out = inc;
    };
    lc("e1e2b_seq_whole_element_distinct", "g(v[0], v[1].x++): two constant elements: no row",
       [=](Kit& k)
       {
           const Expr* inc {nullptr};
           whole(k, false, "1", &inc);
       });
    lc("e1e2b_seq_whole_element_unknown_fail",
       "g(v[i], v[j].x++): the elements may be one: the path-precise unsequenced row",
       [=](Kit& k)
       {
           const Expr* inc {nullptr};
           whole(k, true, "", &inc);
           k.fail_only("unsequenced", inc->loc, 2U, 0U);
       });
    rc("e1e2b_rej_seq_whole_element_same", "unsequenced modification of",
       [=](Kit& k)
       {
           const Expr* inc {nullptr};
           whole(k, false, "0", &inc);
       });
    // int f(struct P *p) requires(owns(*p)): p[0].x = p[0].x++;  (a pointer base has no static key: the path-precise row)
    lc("e1e2b_seq_pointer_base_row",
       "p[0].x = p[0].x++ through a pointer parameter: the unsequenced row (a pointer base has no Sequencer key, as scalar p[i] since e1a)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}})};
           k.req(f, k.land(owns(k, f, 0), k.lt(k.leaf(k.F(f, 0), 0), N(k, "100"))));
           k.ens(f, owns(k, f, 0));
           const Expr* inner {k.b.inc_dec_store_expr(pplace(k, k.pv(f, 0), N(k, "0"), 0), true, true, k.at())};
           const Expr* outer {k.b.store_expr(pplace(k, k.pv(f, 0), N(k, "0"), 0), inner, k.at())};
           f->body = k.blkc({k.es(outer)});
           k.done(f);
           k.fail_only("unsequenced", outer->loc, 1U, 0U);
       });
    // ---- result splits (split_candidates): a pointer result tied to two arguments by `?:` is split against each array's first leaf of its field ----
    // int *pick(int *a, int *b, int c) requires(owns(*a) && owns(*b)) ensures(owns(*a) && owns(*b) && r == (c != 0 ? a : b));
    // void f(int c) { struct P v[2], w[2]; v[0].x = 1; w[0].x = 2; int *r = pick(&v[0].x, &w[0].x, c); *r = 5; }
    // The case splits are no-op `if (r == X) { }` (completeness only): Silicon verifies the write with or without them (the mutant that drops them survived), so the case pins
    // the splits by TEXT, `text_checks`: one against the first element's `x` and `y` of each of the two arrays
    lc("e1e2b_result_splits_candidates",
       "a result tied to two local arrays of structures by ?: is split against each array's first element's int leaves (pinned by the emitted text; the write verifies)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* g {k.fn("pick", k.ptr(k.i32), {{"a", k.ptr(k.i32)}, {"b", k.ptr(k.i32)}, {"c", k.i32}})};
           k.req(g, k.land(owns(k, g, 0), owns(k, g, 1)));
           k.ens(g, k.land(k.land(owns(k, g, 0), owns(k, g, 1)),
                           k.eq(k.R(g), k.cond(k.ne(k.F(g, 2), N(k, "0")), k.F(g, 0), k.F(g, 1)))));
           k.trusted(g);
           k.done(g);
           Function* f {k.fn("f", k.vd, {{"c", k.i32}})};
           Var* v {arr(k, f, P, 2U, false)};
           Var* w {f->new_local("w", k.prog.array_of(k.st(P), 2U), k.at())};
           f->mark_address_taken(w);
           Var* r {k.local(f, "r", k.ptr(k.i32))};
           f->body = k.blkc({k.decl(v, nullptr), k.decl(w, nullptr), k.store(place(k, v, N(k, "0"), 0), N(k, "1")),
                             k.store(place(k, w, N(k, "0"), 0), N(k, "2")),
                             k.decl(r, k.call(g, {place(k, v, N(k, "0"), 0), place(k, w, N(k, "0"), 0), k.pv(f, 0)})),
                             k.store(k.v(r), N(k, "5"))});
           k.done(f);
           k.text_checks.push_back(
               [](const std::string& vpr) -> std::string
               {
                   // `if (t__2 == loc(member(v_v__1, 0), 0)) { }` and `.., 1), 0` for v and for w: four distinct splits of the result against the leaves
                   static const std::regex split {
                       R"(if \(t__[0-9]+ == loc\(member\((v_[a-z]+__[0-9]+), ([01])\), 0\)\) \{ \})"};
                   std::set<std::string> seen;
                   for (std::sregex_iterator it {vpr.begin(), vpr.end(), split}, end; it != end; ++it)
                   {
                       seen.insert((*it)[1].str() + "." + (*it)[2].str());
                   }
                   return seen.size() == 4U
                              ? ""
                              : "expected four case splits of the result against the first leaves of the two arrays, found " +
                                    std::to_string(seen.size());
               });
       });
}

// ---- decision 0064 item 19, audit round 2: clause (iv) (an argument that reaches the cell, handed to any function) and the pair rule (an atom excludes only what it is sure to hold) ----
void define_restrict_fix_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto cell = [=](Kit& k, Function* f, int p, int amount)
    {
        return k.b.atom(AtomKind::Owns, fx(k, f, p), amount, k.at());
    };
    auto ro = [=](Kit& k, Function* f, int p, const Expr* count, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, p), count, amount, k.at());
    };
    auto rr = [=](Kit& k, Function* f, int p, const Expr* count, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, p), count, amount, k.at());
    };
    auto glob = [](Kit& k, const char* name) -> Global*
    {
        Global* g {k.prog.add_global(name, k.i32, k.at(), nullptr)};
        g->init = k.num(k.i32, "0");
        g->value = "0";
        return g;
    };
    auto garr4 = [](Kit& k, const char* name) -> Global*
    {
        Global* g {k.prog.add_global(name, k.prog.array_of(k.i32, 4U), k.at(), nullptr)};
        g->rest = ArrayRest::Zero;
        return g;
    };
    // `<ret> f(<params>)` with `restrict` on the listed parameters, `requires(req)` and `ensures(ens)` built by the callbacks, and a body of the cases' own
    auto restrict_f = [=](Kit& k, const std::vector<Kit::P>& params, const std::vector<int>& restricted,
                          const std::function<const Expr*(Function*)>& req,
                          const std::function<const Expr*(Function*)>& ens) -> Function*
    {
        Function* f {k.fn("f", k.i32, params)};
        for (int r : restricted)
        {
            f->params[static_cast<size_t>(r)].is_restrict = true;
        }
        k.req(f, req(f));
        k.ens(f, ens(f));
        return f;
    };
    // `int callee(int *x)` or the like, with `owns(*x)` in both lists
    auto owning_callee = [=](Kit& k, const std::string& name, const Type& param, bool with_body) -> Function*
    {
        Function* c {k.fn(name, k.vd, {{"x", param}})};
        if (param.is_ptr() && param.pointee != nullptr && *param.pointee == k.i32)
        {
            k.req(c, cell(k, c, 0, 0));
            k.ens(c, cell(k, c, 0, 0));
        }
        if (with_body)
        {
            c->body = k.blk({});
        }
        else
        {
            k.trusted(c);
        }
        k.done(c);
        return c;
    };

    // ---- (iv): the body passes an argument that reaches the cell to ANY function ----
    // int f(int *restrict p) requires(owns(*p)) ensures(owns(*p)) { callee(p); return 0; }: the callee has a body (audit round 2, b1)
    rc("e1e2r4_rej_p_passed_to_a_function_with_a_body", "it calls `callee` with argument 1 `p`",
       [=](Kit& k)
       {
           Function* c {owning_callee(k, "callee", PI(k), true)};
           Function* f {restrict_f(
               k, {{"p", PI(k)}}, {0},
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               },
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               })};
           f->body = k.blk({k.es(k.call(c, {k.b.text(k.pv(f, 0), "p")})), k.ret(N(k, "0"))});
           k.done(f);
       });
    // the same, the callee has no body (audit round 2, issue 3: a trust boundary the rule does not cross)
    rc("e1e2r4_rej_p_passed_to_a_bodiless_function", "it calls `callee` with argument 1 `p`",
       [=](Kit& k)
       {
           Function* c {owning_callee(k, "callee", PI(k), false)};
           Function* f {restrict_f(
               k, {{"p", PI(k)}}, {0},
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               },
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               })};
           f->body = k.blk({k.es(k.call(c, {k.b.text(k.pv(f, 0), "p")})), k.ret(N(k, "0"))});
           k.done(f);
       });
    // the same through an application of a `pure` function that takes the pointer: `check(pf(p) >= 0)`
    rc("e1e2r4_rej_p_passed_to_a_pure_function", "it applies `pf` with argument 1 `p`",
       [=](Kit& k)
       {
           Function* pf {k.fn("pf", k.i32, {{"x", PI(k)}})};
           pf->pure = true;
           pf->pure_body = N(k, "0");
           k.done(pf);
           Function* f {restrict_f(
               k, {{"p", PI(k)}}, {0},
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               },
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               })};
           f->body = k.blk({k.chk(k.ge_c(k.b.apply(pf, {k.b.text(k.pv(f, 0), "p")}, "pf(p)", k.at()), k.i32, "0")),
                            k.ret(N(k, "0"))});
           k.done(f);
       });
    // `&q` of a local `int *q`, an `int **`: the pointer-to-pointer argument (caught by (iii)'s own check of the callee's parameter first, and by (iv))
    rc("e1e2r4_rej_address_of_a_local_pointer_passed", "it calls `callee`",
       [=](Kit& k)
       {
           Function* c {k.fn("callee", k.vd, {{"x", k.ptr(PI(k))}})};
           c->body = k.blk({});
           k.done(c);
           Function* f {restrict_f(
               k, {{"p", PI(k)}}, {0},
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               },
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               })};
           Var* q {k.local(f, "q", PI(k))};
           f->mark_address_taken(q);
           f->body = k.blkc({k.decl(q, k.pv(f, 0)), k.es(k.call(c, {k.addr(q)})), k.ret(N(k, "0"))});
           k.done(f);
       });
    // a pointer to a structure that holds an `int` cell (`struct W { int x; } *`): only (iv) sees it, the callee's parameter is not a pointer to a pointer
    rc("e1e2r4_rej_pointer_to_a_structure_holding_the_cell_passed", "it calls `callee` with argument 1",
       [=](Kit& k)
       {
           StructType* W {k.strukt("W", {{"x", k.i32}})};
           Function* c {k.fn("callee", k.vd, {{"w", k.ptr(k.st(W))}})};
           k.trusted(c);
           k.done(c);
           Function* f {restrict_f(
               k, {{"p", PI(k)}}, {0},
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               },
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               })};
           Var* w {k.scell(f, "w", W)};
           f->body = k.blkc({k.es(k.call(c, {k.addr(w)})), k.ret(N(k, "0"))});
           k.done(f);
       });
    // a structure VALUE with a pointer leaf (`struct H { int *q; }`): only (iv) sees it (the parameter is not a pointer)
    rc("e1e2r4_rej_structure_value_with_a_pointer_leaf_passed", "it calls `callee` with argument 1",
       [=](Kit& k)
       {
           StructType* H {k.strukt("H", {{"q", PI(k)}})};
           Function* c {k.fn("callee", k.vd, {{"h", k.st(H)}})};
           k.trusted(c);
           k.done(c);
           Function* f {restrict_f(
               k, {{"p", PI(k)}}, {0},
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               },
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               })};
           Var* h {k.scell(f, "h", H)};
           f->body = k.blkc({k.es(k.call(c, {k.deref(k.addr(h))})), k.ret(N(k, "0"))});
           k.done(f);
       });
    // the positive: a call whose arguments are scalars is not (iv): `int inc(int x) requires(0 <= x && x <= 100)`; `*p = inc(3); return *p`
    lc("e1e2r4_restrict_function_calls_a_scalar_function",
       "restrict beside a call that passes only an int: accepted and verified",
       [=](Kit& k)
       {
           Function* inc {k.fn("inc", k.i32, {{"x", k.i32}})};
           k.req(inc, k.range(k.F(inc, 0), k.i32, "0", "100"));
           k.ens(inc, k.eq(k.R(inc), k.add(k.F(inc, 0), N(k, "1"))));
           inc->body = k.blk({k.ret(k.add(k.pv(inc, 0), N(k, "1")))});
           k.done(inc);
           Function* f {restrict_f(
               k, {{"p", PI(k)}}, {0},
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               },
               [&](Function* f)
               {
                   return cell(k, f, 0, 0);
               })};
           f->body = k.blk({k.store(k.pv(f, 0), k.call(inc, {N(k, "3")})),
                            k.chk(k.eq_c(k.deref(k.pv(f, 0)), k.i32, "4")), k.ret(N(k, "0"))});
           k.done(f);
       });

    // ---- the pair rule: both surely non-empty, or both ranges with structurally equal counts ----
    // the five shapes of audit round 2 (section 5), as requires/ensures of `int f(...)`; the body touches nothing, as the rejection is of the contract
    // a1c: owns(*p) && owns_range(q, n), `restrict p`
    rc("e1e2r4_rej_pair_cell_and_range_of_any_count", "owns_range(q, n)` may be empty",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}}, {0},
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, fx(k, f, 2), 1));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, fx(k, f, 2), 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // a2c: the same with `reads_range`
    rc("e1e2r4_rej_pair_cell_and_reads_range_of_any_count", "reads_range(q, n)` may be empty",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}}, {0},
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), rr(k, f, 1, fx(k, f, 2), 1));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), rr(k, f, 1, fx(k, f, 2), 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // a3c: `restrict p` backed by owns_range(p, n), beside owns(*q)
    rc("e1e2r4_rej_pair_backing_range_of_any_count_and_cell", "owns_range(p, n)` may be empty",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}}, {0},
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0), cell(k, f, 2, 1));
               },
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0), cell(k, f, 2, 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // a6: two `restrict` ranges of different counts
    rc("e1e2r4_rej_pair_two_ranges_of_different_counts", "the counts `n` and `m` are not structurally equal",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}, {"m", k.i32}}, {0, 2},
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0), ro(k, f, 2, fx(k, f, 3), 1));
               },
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0), ro(k, f, 2, fx(k, f, 3), 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // a6, the counts spelled alike but not equal: `n` and `n + 0`
    rc("e1e2r4_rej_pair_two_ranges_counts_equal_in_value_only", "are not structurally equal",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}}, {0},
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0), ro(k, f, 2, k.add(fx(k, f, 1), N(k, "0")), 1));
               },
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0), ro(k, f, 2, k.add(fx(k, f, 1), N(k, "0")), 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // a7: owns_range(p, n) && owns(g), the body names g: the global's atom does not exempt it
    rc("e1e2r4_rej_pair_range_backing_and_a_held_global", "does not exempt it",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"n", k.i32}}, {0},
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0),
                                 k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 1, k.at(), false));
               },
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0),
                                 k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 1, k.at(), false));
               })};
           f->body = k.blk({k.store(k.b.global_addr(g, k.at()), N(k, "2")), k.ret(N(k, "0"))});
           k.done(f);
       });
    rc("e1e2r4_rej_pair_range_backing_and_a_held_global_text", "may be empty",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"n", k.i32}}, {0},
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0),
                                 k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 1, k.at(), false));
               },
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0),
                                 k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 1, k.at(), false));
               })};
           f->body = k.blk({k.store(k.b.global_addr(g, k.at()), N(k, "2")), k.ret(N(k, "0"))});
           k.done(f);
       });
    // undecided by the record, read as rejecting: a count that is a constant EXPRESSION (2 + 1), not a literal
    // a literal count of 0 holds nothing: `owns(*p) && owns_range(q, 0)`
    rc("e1e2r4_rej_pair_count_is_the_literal_zero", "may be empty",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}}, {0},
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, N(k, "0"), 1));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, N(k, "0"), 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    rc("e1e2r4_rej_pair_count_is_a_constant_expression", "may be empty",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}}, {0},
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, k.add(N(k, "2"), N(k, "1")), 1));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, k.add(N(k, "2"), N(k, "1")), 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // undecided, read as rejecting: `n > 0` is not a top-level conjunct (it is the right arm of `||`), and `m > 0` says nothing of `n`, and `n >= 0` is not positive
    rc("e1e2r4_rej_pair_positive_only_under_an_or", "may be empty",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}}, {0},
               [&](Function* f)
               {
                   return k.land(k.land(cell(k, f, 0, 0), ro(k, f, 1, fx(k, f, 2), 1)),
                                 k.lor(k.eq_c(fx(k, f, 2), k.i32, "5"), k.gt(fx(k, f, 2), N(k, "0"))));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, fx(k, f, 2), 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    rc("e1e2r4_rej_pair_positive_stated_of_another_count", "may be empty",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}, {"m", k.i32}}, {0},
               [&](Function* f)
               {
                   return k.land(k.land(cell(k, f, 0, 0), ro(k, f, 1, fx(k, f, 2), 1)), k.gt(fx(k, f, 3), N(k, "0")));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, fx(k, f, 2), 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    rc("e1e2r4_rej_pair_count_stated_non_negative_only", "may be empty",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}}, {0},
               [&](Function* f)
               {
                   return k.land(k.land(cell(k, f, 0, 0), ro(k, f, 1, fx(k, f, 2), 1)), k.ge(fx(k, f, 2), N(k, "0")));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, fx(k, f, 2), 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // the positives: a1c's shape with `n > 0` as a top-level conjunct, in each of the four spellings; the body writes both pointers and reads `*p` back (permissions exclude the alias)
    const char* forms[] {"n_gt_0", "0_lt_n", "n_ge_1", "1_le_n"};
    for (int form {0}; form < 4; ++form)
    {
        lc(std::string("e1e2r4_restrict_pair_cell_and_range_with_positive_count_") + forms[form],
           "int f(int *restrict p, int *q, int n) requires(owns(*p) && owns_range(q, n) && <n positive>): the pair holds, the body writes q[0] and reads *p",
           [=](Kit& k)
           {
               auto positive = [&](Function* f) -> const Expr*
               {
                   const Expr* n {fx(k, f, 2)};
                   switch (form)
                   {
                       case 0:
                           return k.gt(n, N(k, "0"));
                       case 1:
                           return k.lt(N(k, "0"), n);
                       case 2:
                           return k.ge(n, N(k, "1"));
                       default:
                           return k.le(N(k, "1"), n);
                   }
               };
               Function* f {restrict_f(
                   k, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}}, {0},
                   [&](Function* f)
                   {
                       return k.land(k.land(cell(k, f, 0, 0), ro(k, f, 1, fx(k, f, 2), 1)), positive(f));
                   },
                   [&](Function* f)
                   {
                       return k.land(cell(k, f, 0, 0), ro(k, f, 1, fx(k, f, 2), 1));
                   })};
               f->body = k.blk({k.store(k.pv(f, 0), N(k, "1")),
                                k.store(k.b.elem_addr(k.pv(f, 1), N(k, "0"), k.at()), N(k, "2")),
                                k.chk(k.eq_c(k.deref(k.pv(f, 0)), k.i32, "1")), k.ret(N(k, "0"))});
               k.done(f);
           });
    }
    // a2c's shape with `n > 0`: `reads_range(q, n)` beside owns(*p)
    lc("e1e2r4_restrict_pair_cell_and_reads_range_with_positive_count",
       "owns(*p) && reads_range(q, n) && n > 0: the pair holds",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}}, {0},
               [&](Function* f)
               {
                   return k.land(k.land(cell(k, f, 0, 0), rr(k, f, 1, fx(k, f, 2), 1)), k.gt(fx(k, f, 2), N(k, "0")));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), rr(k, f, 1, fx(k, f, 2), 1));
               })};
           Var* t {k.local(f, "t", k.i32)};
           f->body =
               k.blkc({k.store(k.pv(f, 0), N(k, "1")), k.decl(t, k.deref(k.b.elem_addr(k.pv(f, 1), N(k, "0"), k.at()))),
                       k.chk(k.eq_c(k.deref(k.pv(f, 0)), k.i32, "1")), k.ret(N(k, "0"))});
           k.done(f);
       });
    // an `owns_block` covering `q` pairs with nothing (undecided by the record, read as rejecting): owns(*p) && owns_block(q) beside `restrict p`
    rc("e1e2r4_rej_pair_cell_and_owns_block", "owns_block(q)` is not a cell or structure atom",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}}, {0},
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), k.b.atom(AtomKind::OwnsBlock, fx(k, f, 1), -1, k.at()));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), k.b.atom(AtomKind::OwnsBlock, fx(k, f, 1), -1, k.at()));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // a3c's shape with `n > 0`: `restrict p` backed by owns_range(p, n) beside owns(*q)
    lc("e1e2r4_restrict_pair_backing_range_with_positive_count_and_cell",
       "owns_range(p, n) && n > 0 beside owns(*q): the pair holds",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}}, {0},
               [&](Function* f)
               {
                   return k.land(k.land(ro(k, f, 0, fx(k, f, 1), 0), cell(k, f, 2, 1)), k.gt(fx(k, f, 1), N(k, "0")));
               },
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0), cell(k, f, 2, 1));
               })};
           f->body = k.blk(
               {k.store(k.b.elem_addr(k.pv(f, 0), N(k, "0"), k.at()), N(k, "1")), k.store(k.pv(f, 2), N(k, "2")),
                k.chk(k.eq_c(k.deref(k.b.elem_addr(k.pv(f, 0), N(k, "0"), k.at())), k.i32, "1")), k.ret(N(k, "0"))});
           k.done(f);
       });
    // literal counts: `owns(*p) && owns_range(q, 4)`; and two ranges of literal counts 2 and 3 (both surely non-empty)
    lc("e1e2r4_restrict_pair_cell_and_range_of_a_literal_count",
       "owns(*p) && owns_range(q, 4): the count is an integer constant of at least 1",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}}, {0},
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, N(k, "4"), 1));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0), ro(k, f, 1, N(k, "4"), 1));
               })};
           f->body =
               k.blk({k.store(k.pv(f, 0), N(k, "1")), k.store(k.b.elem_addr(k.pv(f, 1), N(k, "3"), k.at()), N(k, "2")),
                      k.chk(k.eq_c(k.deref(k.pv(f, 0)), k.i32, "1")), k.ret(N(k, "0"))});
           k.done(f);
       });
    lc("e1e2r4_restrict_pair_two_ranges_of_literal_counts",
       "owns_range(p, 2) && owns_range(q, 3), both restrict: both surely non-empty",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}}, {0, 1},
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, N(k, "2"), 0), ro(k, f, 1, N(k, "3"), 1));
               },
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, N(k, "2"), 0), ro(k, f, 1, N(k, "3"), 1));
               })};
           f->body = k.blk({k.store(k.b.elem_addr(k.pv(f, 0), N(k, "1"), k.at()), N(k, "1")),
                            k.store(k.b.elem_addr(k.pv(f, 1), N(k, "2"), k.at()), N(k, "2")),
                            k.chk(k.eq_c(k.deref(k.b.elem_addr(k.pv(f, 0), N(k, "1"), k.at())), k.i32, "1")),
                            k.ret(N(k, "0"))});
           k.done(f);
       });
    // two `restrict` ranges of the SAME count expression `n` (a6's twin; empty together)
    lc("e1e2r4_restrict_pair_two_ranges_of_the_same_count",
       "owns_range(p, n) && owns_range(q, n), both restrict: empty together",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}}, {0, 1},
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 2), 0), ro(k, f, 1, fx(k, f, 2), 1));
               },
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 2), 0), ro(k, f, 1, fx(k, f, 2), 1));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // a single guarded range needs no pair: `p == NULL || owns_range(p, n)`
    lc("e1e2r4_restrict_single_guarded_range_needs_no_pair",
       "p == NULL || owns_range(p, n): no other covering atom, no pair",
       [=](Kit& k)
       {
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"n", k.i32}}, {0},
               [&](Function* f)
               {
                   return k.lor(k.eq(fx(k, f, 0), k.nul(PI(k))), ro(k, f, 0, fx(k, f, 1), 0));
               },
               [&](Function* f)
               {
                   return k.lor(k.eq(fx(k, f, 0), k.nul(PI(k))), ro(k, f, 0, fx(k, f, 1), 0));
               })};
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // a7's shape with `n > 0`: owns_range(p, n) && owns(g) && n > 0, the body writes both
    lc("e1e2r4_restrict_range_with_positive_count_beside_a_held_global",
       "owns_range(p, n) && owns(g) && n > 0: the global's atom exempts g, the branch p == &g is dead",
       [=](Kit& k)
       {
           Global* g {glob(k, "g")};
           Function* f {restrict_f(
               k, {{"p", PI(k)}, {"n", k.i32}}, {0},
               [&](Function* f)
               {
                   return k.land(k.land(ro(k, f, 0, fx(k, f, 1), 0),
                                        k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 1, k.at(), false)),
                                 k.gt(fx(k, f, 1), N(k, "0")));
               },
               [&](Function* f)
               {
                   return k.land(ro(k, f, 0, fx(k, f, 1), 0),
                                 k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 1, k.at(), false));
               })};
           f->body = k.blk({k.store(k.b.elem_addr(k.pv(f, 0), N(k, "0"), k.at()), N(k, "1")),
                            k.store(k.b.global_addr(g, k.at()), N(k, "2")),
                            k.chk(k.eq_c(k.deref(k.b.elem_addr(k.pv(f, 0), N(k, "0"), k.at())), k.i32, "1")),
                            k.ret(N(k, "0"))});
           k.done(f);
       });
    // `owns(g)` of the global array `int g[4]` (a range atom of the constant count 4: audit round 2, issue 4) exempts it, beside `owns(*p)`
    lc("e1e2r4_restrict_beside_a_held_global_array",
       "owns(*p) && owns(g) with int g[4]: the whole-array atom is surely non-empty and exempts g",
       [=](Kit& k)
       {
           Global* g {garr4(k, "g")};
           Function* f {restrict_f(
               k, {{"p", PI(k)}}, {0},
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0),
                                 k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 1, k.at(), false));
               },
               [&](Function* f)
               {
                   return k.land(cell(k, f, 0, 0),
                                 k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 1, k.at(), false));
               })};
           f->body = k.blk({k.store(k.pv(f, 0), N(k, "1")),
                            k.store(k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, "0"), k.at()), N(k, "2")),
                            k.chk(k.eq_c(k.deref(k.pv(f, 0)), k.i32, "1")), k.ret(N(k, "0"))});
           k.done(f);
       });
}

} // namespace
