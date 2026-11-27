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
    lc("e1e2a_guard_object_view",
       "a structure local whose member is an array of structures (Part A's guard of the object view, lifted by Part C2 slice 1 for the allocation, the scope exit and the atoms)",
       [=](Kit& k)
       {
           // void f(void) { struct S s; }  with struct S { struct T a[2]; int n; }
           StructType* S {S_agg(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           f->body = k.blkc({k.decl(s, nullptr)});
           k.done(f);
       });
    lc("e1e2a_guard_value_view",
       "a by-value structure parameter whose member is an array of structures (Part A's guard of the value view, lifted by Part C2 slice 3: the value view expands the member)",
       [=](Kit& k)
       {
           // void f(struct S s) {}
           StructType* S {S_agg(k)};
           Function* f {k.fn("f", k.vd, {{"s", k.st(S)}})};
           f->mark_address_taken(f->param_var(0));
           f->body = k.blk({});
           k.done(f);
       });

    // ---- trap 1: an element of a row is not a flat element of a pointer ----
    // (slice 3a lifted the guard for a global matrix: the element of its row is a named element; a function that holds nothing of `g` fails the permission row)
    lc("e1e2a_guard_elem_addr_of_a_row",
       "the address of a row of a global matrix: g[1][2] read by a function that holds nothing of g",
       [=](Kit& k)
       {
           // int g[2][3]; int f(void) { return g[1][2]; }
           Global* g {matrix(k)};
           Function* f {k.fn("f", k.i32)};
           const Expr* rd {cell_of_matrix(k, g, "1", "2")};
           f->body = k.blk({k.ret(rd)});
           k.done(f);
           k.fail("permission", rd->loc);
           k.rows("uninitialized", rd->loc, 1U);
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
    lc("e1e2a_guard_inv_elem_of_a_row",
       "g[1][2] in the invariant of a loop of a function that holds nothing of g: unframed (permission at the read)",
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
           k.fail_only("permission", rd->loc, 2U, 0U, "this loop invariant reads");
       });

    // ---- trap 2: element keys of a base that holds an index of its own ----
    // (Part E slice 2b lifted trap 2 for a row parameter: the case that pinned its internal error is `e1e2e_rowparam_contract_reads_keys`, which checks that every index is in the key)

    // Part C1 (slice 2a) lifts this guard too: the call rows of a range over structures (item 8), the access of its callee per leaf path (flipped from `ru`)
    lc("e1e2a_guard_range_over_structures_in_a_call",
       "a call of a function that needs owns_range(p, n) over struct T, passing its own range: the call rows per leaf path",
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
    // Part C1 lifts this guard (the contract of one function: item 7's rows per leaf path; flipped from `ru`); a call of such a function is the case above (slice 2a)
    lc("e1e2a_guard_range_over_structures",
       "owns_range(p, n) over struct T in a function that does nothing: the rows of its leaf path in the contract",
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
    // Part D (step B) builds a written `owns(*q)`; the `reads` form has no fixture (S48 (k)) and stays refused
    rc("e1e2a_rej_invariant_owns_of_a_structure_with_an_array_member",
       "a `reads` atom on a structure with an array member in a loop invariant",
       [=](Kit& k)
       {
           // void f(struct U *p) requires(owns(*p)) ensures(owns(*p)) { struct U *q = p; int i = 0; while (i < 3) invariant(reads(*q) && 0 <= i && i <= 3) decreases(3 - i) { i = i + 1; } }
           StructType* U {U_arr(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(U))}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           Var* q {k.local(f, "q", k.ptr(k.st(U)))};
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* loop {
               k.whl(k.lt(k.v(i), N(k, "3")),
                     {k.cl(k.b.atom(AtomKind::Reads, k.v(q), 1, k.at())), k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                     {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({k.asg(i, k.add(k.v(i), N(k, "1")))}))};
           f->body = k.blkc({k.decl(q, k.pv(f, 0)), k.decl(i, N(k, "0")), loop});
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
    rc("e1e2a_rej_marked_member_in_an_array_element",
       "has an array member of structures or arrays and an `owning` or `ref_to_uninit` member",
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
    for (const std::vector<StructLeaf>& view : {struct_leaves(*W), struct_object_leaves(*W)})
    {
        expect(
            view.size() == 2U && view[0U].aggregate_element && view[0U].type.is_array() && !view[1U].aggregate_element,
            "a view of a structure with an array of structures has the flagged member and the cell n, and no expansion");
    }
    expect(struct_object_leaves(*W)[0U].array && struct_object_leaves(*W)[0U].element == k.st(V),
           "the object view keeps `array` and the aggregate element");
    // decision 0064 item 12 (Part C2): the VALUE view expands a member that is an array of structures, element by element, each element's leaves in declaration order: `path` through the
    // array member to the leaf, `elem` the index, `index_after` where the index step stands
    StructType* Z {
        k.strukt("Z", {{"v", k.prog.array_of(k.st(k.strukt("PZ", {{"x", k.i32}, {"y", k.i32}})), 2U)}, {"n", k.i32}})};
    const std::vector<StructLeaf> zv {struct_value_leaves(*Z)};
    expect(
        zv.size() == 5U && !zv[0U].aggregate_element && zv[0U].path == std::vector<int> {0, 0} && zv[0U].elem == 0 &&
            zv[0U].index_after == 1 && zv[1U].path == std::vector<int> {0, 1} && zv[1U].elem == 0 &&
            zv[2U].path == std::vector<int> {0, 0} && zv[2U].elem == 1 && zv[3U].path == std::vector<int> {0, 1} &&
            zv[3U].elem == 1 && zv[4U].path == std::vector<int> {1} && zv[4U].elem == -1 && zv[4U].index_after == -1 &&
            zv[3U].text == "v[1].y",
        "the value view of a member that is an array of structures: every leaf of every element, element by element");
    // a matrix member (decision 0064 item 12, Part E slice 4a): the value view of a type of depth 2 is value_cells' list, cell for cell, never flagged: six leaves, each with its steps
    StructType* X {k.strukt("X", {{"m", k.prog.array_of(k.prog.array_of(k.i32, 3U), 2U)}})};
    {
        const std::vector<StructLeaf> xv {struct_value_leaves(*X)};
        expect(xv.size() == 6U && !xv[0U].aggregate_element && xv[4U].text == "m[1][1]" &&
                   xv[4U].steps == std::vector<ValueStep> {{false, 0U}, {true, 1U, true}, {true, 1U}} &&
                   xv[4U].path == std::vector<int> {0} && xv[4U].elem == 1 && xv[4U].index_after == -1,
               "a matrix member: six leaves, m[1][1] with the steps member 0, element 1 (of rows), element 1");
    }
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
    //   while (i < n) inv(0 <= i <= n) dec(n - i) { v[i].x = i; [v[i].y = 5;] i = i + 1; } check(v[1].y == 4 && v[2].y == 6 [with the write: check(v[2].y == 6) alone]); return i; }
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
        // the failing variant checks one fact (v[2].y == 6): the conjunction would give two failing rows at one line, and scripts/run_tests.py counts errors exactly
        const Stmt* c {k.chk(
            write_y ? k.eq(read(k, v, N(k, "2"), 1), L(k, "6"))
                    : k.land(k.eq(read(k, v, N(k, "1"), 1), L(k, "4")), k.eq(read(k, v, N(k, "2"), 1), L(k, "6"))))};
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

} // namespace
