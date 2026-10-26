# 0024 — Values the front end computes: static assertions, character literals, types inside types (M4.4b2)

- **Status:** accepted for M4.4b2 (2026-10-26). Owner decisions (handoff §11 questions 32 and 39, M4.4b2 planning):
  - **Question 32:** "Fix it now, and any other known instances": every place where EDG computes an integer value
    alone, wraps it, and cerastes trusts the result is closed, by encoding the value as written or by rejecting the
    construct. `#if` is the exception: it needs an EDG fork commit and goes to the EDG box session (handoff §8.1).
  - **File-scope static assertions** are reached by turning on EDG's source sequence lists, after a spike (P45).
  - **Question 39:** a static assertion whose condition cerastes cannot encode is **skipped**, not rejected, "for now";
    type traits are handled in a new session before M5 (**M5.0**, `plan.md`).
  - **Array types inside types** (`sizeof`, `_Alignof`, `typeof`, `decltype` operands, typedefs) are **rejected until
    M4.5e**, not checked (2026-10-26).
  - **Expectation changes:** none. The Viper of `static_assert_ok.c` and `static_assert_overflow.c` changes; their
    verdicts do not. Any verdict change stops the work.

  Facts marked (P*n*) are probes, recorded with their evidence in `internal/il-inventory.md`, "M4.4b2" (P42–P45).
  Planning cut the optional split of a top-level `&&` in a skipped assertion (item 3).

## Context

Record 0022 made every integer operation exact on mathematical integers: a result that does not fit its type is an
error, never a wrap. It listed the values EDG computes alone, where wrapping still happens (0022 item 2): `#if`,
`_Static_assert`, array bounds and other expressions inside types, template arguments, and character literals.
`_Static_assert(65536u * 65536u == 0u, "")` verifies, `'\xff'` is the `char` -1 (an `int` -1 in C), and
`sizeof(char[65536u * 65536u + 1])` is 1. These are C's own values, so cerastes was not unsound, but it accepted
programs that rely on a wrap, which 0022 forbids everywhere else. Template arguments are closed (P38: no `expr`, so
rejected).

## Decision

1. **Source sequence lists** (amends 0002; P45). The cerastes configuration sets
   `GENERATE_SOURCE_SEQUENCE_LISTS` to 1 in `cmake/EdgFrontEnd.cmake`; EDG derives `GENERATE_LINKAGE_SPEC_BLOCKS` and
   `GENERATE_MICROSOFT_IF_EXISTS_ENTRIES` (1) and its two template-instantiation variants (0) from it, and all five are
   asserted in `src/backend/edg_il.h` and checked on the binaries by `scripts/check_edg_config.sh`. EDG's IL walk then follows the
   scopes' source sequence lists, which also reach each redeclaration (a secondary-declaration entry) with copies of
   its attribute entries and its declared type. The walker (`--cerastes_walk`) does not list what only a secondary
   declaration reaches (owner, 2026-10-26: the listing keeps its meaning, and no golden changes; the round 1 audit
   preferred listing them, which would change two goldens): the attribute copies of a secondary-declaration entry, and
   an unrecognized attribute on a function type that the walk reaches but no routine prints (a redeclaration's type;
   the rule is by type kind, since skipping every `declared_type` also hid a repeated typedef's real attribute,
   `il/contrast/alias_attribute.cpp`); a registered `cerastes::` attribute there is still listed, so none is lost. With
   that, the walker's goldens, the walker oracle and the emitted Viper of every existing test are unchanged (P45: before
   the walker change 28 goldens gained `<unknown owner>` lines and 2 gained a redeclaration's attributes; every `.vpr`
   was identical). **The encoder sees a redeclaration's attributes now** (amends 0011 item 7): the type of a
   redeclaration is among the types whose attributes are held to the allow-list, so `int g(int); int g(int x)
   [[unreg_ns::a2(x > 0)]]; int g(int x) {…}`, accepted before, is rejected; an attribute written on a redeclaration
   was never checked until now. The encoder reads only the static assertions of
   the file scope's list: file, namespace, `extern "C"` and class scope, flat and in source order; a block-scope
   assertion is never on it, and those of system headers are, filtered by position (P43).
2. **Static assertions are checked as written.**
   - **Where.** A block-scope `static_assert`/`_Static_assert` (the `iek_static_assertion` entity of a declaration
     statement) is checked in its function's prologue, and in its `__wf` method, like a case value (0017 item 5): local
     constants and block assertions in the order they are met, then the case values. A file-scope or namespace-scope
     one (the file scope's source sequence list) is checked in every method's prologue, in source order with the named
     constants and enumerators (0017 item 4, 0018). An assertion is in a verified file when its position, or its
     condition's original expression's position, is.
   - **How.** The condition constant is encoded like any constant the encoder reads (`constant_value`: its original
     expression when it has one, else the literal, so a lone wrapped `'\xff'` is item 5's error), then taken as a truth
     value (`clause_bool`): in C the condition keeps its own type (`static_assert(4)` is the `int` 4,
     `static_assert(sizeof(int))` an `unsigned long`), in C++ it is `bool`, with a generated `bool` conversion at the root
     of an integer condition (P43). Its obligations are asserted at their own positions, then an `internal` guard asserts
     that the condition as written holds (EDG accepted it, so it holds in EDG's arithmetic). A failing obligation ends the
     prologue's path (0017 item 4), so a condition that relies on a wrap is reported as `overflow`, `shift` or
     `conversion`, and a disagreement with EDG is exit 3. A lone literal or enumerator (`static_assert(1)`,
     `_Static_assert(N, "")`) has nothing to check (an enumerator's initializer is checked by 0018). Every non-literal
     condition keeps its `expr` in both modes (P43: 52 shapes). A failed assertion is an EDG error (#1574 in C, #2784 in
     C++), so the back end never sees its NULL condition; the encoder rejects one anyway. EDG refuses `(1 << 40) == 0`
     and `2147483647 + 1 < 0` in C++ (#28) but folds them with the wrap in C, where the encoder is now the only check.
     A non-integer condition (`static_assert(1.5)`, `nullptr == nullptr`) is rejected with its type.
   - **Names.** A C++ local `constexpr` named in a block assertion is a named constant of its function (0017 item 6), by
     the case-value path; in C, a condition that names an object is rejected (defence in depth for #1901, 0017 item 2).
   - **Bodies that are not encoded.** Static assertions in `predicate` and `pure` bodies (0011 item 4) are not checked
     until M4.5f, as for case values (0017 item 5).
   - **Class scope and the cross-check.** A class-scope assertion is on the file list and is encoded like the others
     (classes are rejected in verified files anyway); a dependent condition (`ck_template_param`, in a class template,
     which is rejected) is left alone. Fail closed: every static assertion of a verified file that EDG's walk of the file
     scope reaches must be encoded, skipped by item 3, or dependent; any other is rejected. Inside functions the statement
     walk of every encoded body reaches each `stmk_decl` (P43: nested blocks, dead code, after `return`); bodies that are
     not encoded hold assertions that are not checked: rejected functions (their file is rejected) and `predicate`/`pure`
     bodies (above). A class local to a function, the only other holder (a sublist of the function's list), is rejected.
     The round 2 audit asked for the comparison in function regions too; it is a handoff note. An assertion whose keyword
     and condition have no position counts as in a verified file.
   - **Self-test.** `perturb_folded` (0017 item 9) also flips each static assertion's guard.
3. **The skip (question 39).** Before encoding, the condition is scanned, following folded constants' expressions but
   not entering the operands of `sizeof`, `_Alignof`, `__datasizeof` and `noexcept`, which are never evaluated (they go
   through the encoder and item 6). If it contains any of these expression nodes, the whole assertion is not encoded,
   silently (P43 names each):
   - a call (`eok_call` of an `enk_routine`) of a function declared in a system file, or of a builtin without a
     position: `std::numeric_limits<int>::max()`, `__builtin_constant_p`. A call of a user function is not a trigger:
     the encoder rejects it (a user `constexpr` function is itself rejected today, 0012), so
     `_Static_assert(sizeof(g()) == sizeof(int) && 65536u * 65536u == 0u, "")` is not skipped: it is rejected, by 0020
     item 5's rule for a call inside `sizeof` (`side_effect_not_evaluated`) (round 1 audit);
   - a builtin operation (`enk_builtin_operation`): the type traits such as `__is_integral` and `__is_same`, and
     `__builtin_offsetof`;
   - a concept-id (`enk_concept_id`): `std::integral<int>`;
   - a variable declared in a system file (`enk_variable`): `std::is_integral_v<int>`, `std::is_integral<int>::value`,
     `std::numeric_limits<int>::digits`;
   - an enumerator declared in a system file (C's `_CLOCK_REALTIME`).

   Types never trigger the skip (`sizeof(size_t)` is encoded). Any other node goes to the encoder and gets its
   rejections, warning shapes (item 4) and the type rule (item 6) included. **A skipped assertion is still held to the
   type rule** (round 1 audit): every type operand of a builtin operation (`__is_same(char[65536u * 65536u + 1],
   char[1])`, `__builtin_types_compatible_p`) and the template arguments of every entity the condition names (a
   variable template's own arguments, `std::is_same_v<char[…], char[1]>`; the class instance holding a static member
   or member function, `std::integral_constant<unsigned, 65536u * 65536u>::value`) go through item 6's helper, which
   rejects a non-type template argument. EDG still checks a skipped assertion in its own arithmetic. The gap — a wrap
   inside a skipped condition's own operations is not reported — is documented and closes in M5.0. A top-level `&&` is
   not split (cut in planning).
4. **Warning shapes** (amends 0023 item 5). The detectors of 0020 item 5 and 0023 item 5 run on encoded static
   assertion conditions (P41 found `~true` and `2 ^ 8` warned there).
5. **Wrapped character literals** (amends 0022 items 2 and 3; P42).
   - **Shape.** A character literal holds no `expr` and no `orig_type`; a plain one is sign-extended from `char` (an
     `int` in C, a `char` in C++), and an `L` one from `wchar_t` (an `int` in C, `wchar_t` in C++; 32 bits, signed, on
     macOS). `u`, `U` and `u8` literals are unsigned and never negative (C23 `u8'\xff'` is the `unsigned char` 255).
     Integer literals are never negative and every folded constant keeps its `expr` (P38), so P42's census finds three
     shapes of negative integer constant with neither `expr` nor `orig_type`: character literals; enumerators without a
     written initializer (an enumerator use, flagged, read through 0018); and non-type template arguments (0022 item 3,
     closed while templates are rejected; the helper never reads them, a note for M5). The round 1 audit found a fourth:
     copies of an enumerator's value typed by its enumeration (C++ `enum E y = A;`, `enum EI : int { IM = -2 }; enum EI
     x = IM;` in both modes, a C23 `case IM:` of a fixed enumeration of rank at least `int`), which keep today's reading.
     So the rule is by value **and type**: a negative, non-enumerator constant with neither, whose type is plain `int` (C,
     not an enumeration) or `char`/`wchar_t` (C++), is a character literal whose code did not fit; any other shape is
     read as before. Its width N is 32 when `character_kind` is
     `chk_wchar_t` or the value is below -128, else 8. N only shapes the message: converting `v + 2ᴺ` into a signed
     N-bit type fails for every such value. (`character_kind` is kept on the literal and on its expression node, but
     not on converted copies, which have an `expr`, nor on the enumerators of a fixed enumeration, P42.)
   - **Encoding.** Such a literal is the conversion of its code `v + 2ᴺ` (an unsigned N-bit constant) into its
     character type, then, in C, into `int` (which adds no obligation). The conversion fails for every such code, so the
     literal is a `conversion` error at its position, on the paths that reach it: `char c = '\xff';`, `(unsigned
     char)'\xff'`, `'\377'`, `L'\xffffffff'`. For an enumerator (below) the character type comes from
     `character_kind` (`wchar_t`, or `int` in C, for `chk_wchar_t`, else `char`), not from the constant's type, which
     in C++ is the enumeration: so `enum { E = '\xff' };` is the same `conversion` error in both modes.
   - **Where.** Everywhere the encoder reads a literal (`constant_value`): bodies, contracts, annotations, case values
     (in the prologue, at the case label), named-constant initializers, default arguments (at the call, 0022 item 2), and
     macros (at the use, with a note at the definition); and an enumerator whose written initializer is such a literal
     (`enum { E = '\xff' };`), which was taken without a check: it is checked at the enumerator in the prologue. An
     implicit enumerator after `-2` has the same constant shape (the `int` -1, no `expr`, flagged); only the written
     initializer (`enum_value_range`, non-null) tells them apart (P42).
   - **Truncated escapes** (`'\x100'`, `'\400'`, `L'\x100000000'`) and too-large integer literals are already errors
     in verify mode (#27, #23: EDG makes them warnings only in GNU emulation, which verify mode refuses; P42). Tests pin
     them; nothing changes in the driver. C23's truncated `u'\x10000'` (#3255) is 0023 item 8's.
   - **Rewrites:** `u8'\xff'` (C23 `unsigned char`), `(unsigned char)0xff`, `(char)-1`.
6. **Types inside types are rejected until M4.5e** (amends 0010 item 13 and 0018 item 10; P44).
   - **The rule.** A type is *computed from a constant* when it is, or contains through typedefs, `typeof`/`decltype`
     and alias typerefs, pointers, arrays (`element_type`), function types (the return type and each parameter's
     declared type) and vector element types, an array type, a `_BitInt(N)` type (`ik_bit_precise`,
     `ik_unsigned_bit_precise`), a vector type (`tk_vector`), or an instance of a class template with a non-type
     template argument (`tak_nontype`: `std::array<char, 4>`, `std::bitset<8>`); for a class instance, each type
     template argument goes through the same rule; the members of other classes are not entered (classes in verified
     files are rejected anyway, 0012) (P44).
   - **Where.** Such a type is rejected (`unsupported_type`) as the type operand of `sizeof`, `_Alignof` or
     `__datasizeof`, as the type of their expression operand (so `sizeof((char[4]){0})` and `sizeof("abc")`, whose value
     depends on the operand's array type, are rejected until M4.5e and M4.6), in every place a constant is read
     (bodies, contracts, named constants, case values, enumerators, default arguments, static assertions). The types of
     the nodes inside an expression operand do not decide its value and are not scanned. In a C enumerator 0018 item
     10's rule applies first (one diagnostic). `_Alignas`/`alignas` is an attribute, already outside 0011 item 7's
     allow-list (P44).
   - **Typedefs.** A typedef (or C++ alias) declared in a verified file whose type contains an array type is rejected
     (`unsupported_declaration`), at block and file scope, used or not. A vector typedef stays accepted until it is
     used (`reject_vector.c`). A rejected typedef used in `sizeof` reports both lines.
   - **Declared types** (round 1 audit). A type the encoder reads as a value type (a local's, a parameter's, a
     result's, a cast's) or as an enumeration's written underlying type (round 2) that reaches its meaning through an
     alias-template instance or a member typedef of a class instance with a non-type template argument is rejected
     (`unsupported_type`), once, at the declaration: `std::conditional_t<(65536u * 65536u == 0u), int, long> y = x;`
     chooses `int` by a wrapped constant. The walks give up after 64 levels by rejecting ("a type nested too deeply"),
     never by accepting (round 2).
   - `sizeof(int[3])` is rejected too, until M4.5e encodes arrays.
7. **What is left.** `#if` (the EDG box session); static assertions skipped by item 3 (M5.0); the bodies of `predicate`
   and `pure` functions (M4.5f).

## Amends

- 0002: the configuration table gains `GENERATE_SOURCE_SEQUENCE_LISTS` and its two derived macros.
- 0010 item 13: static assertions are encoded; expressions inside types are rejected.
- 0017 items 4, 5, 6 and 9: static assertions in the prologue, the local-constant path, the self-test hook.
- 0018 item 10: the type rule outside C enumerators; the enumerator gaps (a literal initializer, a top-level `sizeof`).
- 0022 items 2 and 3: character literals do reach the literal arm; they are now checked.
- 0023 item 5: the detectors run in static assertions.

## Consequences

- Static assertions in verified files are checked in cerastes's arithmetic; one that relies on a wrap is an error.
- A plain character literal above `'\x7f'` is a `conversion` error on the paths that reach it.
- Programs with array types in `sizeof` or array typedefs are rejected until M4.5e.
- Programs accepted before and rejected now (each with a test): a static assertion whose condition is not an integer
  (`_Static_assert(1.5, "")`) or holds a construct the encoder rejects (`noexcept`, `_Generic`, a string, a user call);
  `sizeof("abc")`; an attribute outside the allow-list on a redeclaration (item 1); a declared type chosen by a non-type
  template argument (item 6); a skipped assertion whose types or template arguments the type rule rejects (item 3). A
  static assertion before a `switch`'s first case label is rejected by 0014's rule for statements there.
- The emitted Viper of programs without these constructs is unchanged.
