# 0035 — `member_inv`: a member block determines its parent and its index (the `Block` domain's axioms, M4.5b1a)

- **Status:** accepted for M4.5b1a (2026-10-28). Record 0028 item 2 (owner, 2026-10-27, after S24 and S26) chose the
  `Block` domain with four axioms and said that each gets its own record and lint tag when M4.5b1 implements it.
- **Milestone:** M4.5b1a (2026-10-28); emitted from M4.5b1a by every program that has a pointer type or an address-taken
  local (record 0032 item 5).
- **Part of:** record 0028 item 2's domain; the other axioms are 0033, 0034, 0036.

## Decision

```
axiom member_inv { forall r: Ref, k: Int :: { member(r, k) } parent_of(member(r, k)) == r && index_of(member(r, k)) == k }
```

Each struct member and inner array is a block of its own (0028 item 16, owner after audit round 2). The axiom makes
member blocks of different objects, and different members of one object, distinct, and their receivers injective under
quantifiers. **Needed** (S26): without it 7 of 13 files change verdict (arrays of structs, nested structs). Nothing in
M4.5b1a builds a `member` term (structs are M4.5c), so the axiom is emitted with the domain but never triggered there.

Triggered only by `member(r, k)`.

**The model** (0028 item 2): `Ref` is the free term algebra `null` | `loc(Block, Int)`, `Block` is the allocated blocks
| `member(Ref, Int)`; `block_of`, `offset_of`, `parent_of`, `index_of` are the projections; `lvloc(b)` is `loc(b, -2)`
for an allocated block and `lvloc(block_of(r))` for `member(r, k)`, by recursion on the term; `len` is arbitrary. Every
one of the four axioms holds in it for every argument, with no guard, and every trusted method (`alloc_T`, record 0032
item 4; later `program_start` and vlibc's allocations) states only facts true of a fresh allocated block in it. **Runs**
(`il-inventory.md`, S24, S26 final): `e2_consistency_terms_fail.vpr` instantiates the terms of every axiom and then
asserts `false`, failing with exactly that one error for each of the 16 subsets of the four axioms;
`e2_consistency_calls_fail.vpr` does the same after every trusted method; `vacuity2.py` found all 139 points (method
starts, ends, after every trusted call) of the regenerated fixtures reachable.

**Lint** (record 0032 item 11): the line before the axiom is `// cerastes:axiom-ok 0035`, and the domain is preceded by
`// cerastes:domain-ok 0028`; `lint_viper.py` refuses an axiom without its tag or with a tag naming a record that does
not exist, and any other domain or axiom.

## Consequences

- One of exactly four axioms in cerastes's Viper (with 0033–0036); any other is refused by the lint.
- Changing it changes the fixtures in `tests/viper-smoke/memory/`, which change only with a record.
