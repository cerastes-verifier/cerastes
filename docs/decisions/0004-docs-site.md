# 0004 — User documentation site

- **Status:** accepted
- **Milestone:** M1.5 (2026-10-14)

## Context

plan.md M1.5 asks for user documentation written in AsciiDoc in `doc/`,
built with Antora, checked by `scripts/check.sh` and published. From M1.5
on, every milestone documents what it makes user-visible, and every code
example in `doc/` is a test (invariant 8 in CLAUDE.md, 9 in plan.md).

plan.md's default hosting was a dedicated org repo `cerastes-verifier/doc`
that holds only a publishing playbook and workflow. It would rebuild on a
`repository_dispatch` from this repo, so that its GitHub Pages project site
appears at `https://cerastes.dev/doc/`. The alternative plan.md asked us to
weigh was building Antora inside the website's own workflow, into
`dist/doc/`.

The owner rejected the docs-only repo (`docs/handoff.md` after M1,
question 4, 2026-10-13): "Use the repo specific .github.io page, and then we
will fix the specific address later." They enabled GitHub Pages on
`cerastes-verifier/cerastes`, with GitHub Actions as the source.

Facts checked on 2026-10-13:
- `cerastes-verifier/cerastes` is public, and the API reports
  `has_pages: true`.
- The org site `cerastes-verifier.github.io` has the custom domain
  `cerastes.dev`. GitHub therefore serves the org's project sites under that
  domain: `https://cerastes-verifier.github.io/cerastes/` answers
  `301 → http://cerastes.dev/cerastes/`.

## Decision

1. **Hosting.** The docs are this repository's GitHub Pages project site,
   deployed by GitHub Actions from this repository. The published address is
   **`https://cerastes.dev/cerastes/`**. It is interim: the owner fixes the
   final address later.
   - `site.url` is set in one place, `doc/local-playbook.yml`.
   - Antora writes relative links, so moving the site means changing that one
     value, plus the URL lines in `CLAUDE.md` and `README.md`. Possible
     targets include a custom domain on this repo, a rename, or a path the
     website serves.
2. **Rejected alternatives.**
   - A docs-only repo: the owner does not want one. It would also have
     needed a cross-repo dispatch token and a nightly backstop.
   - Building inside the website's workflow (`dist/doc/`): every docs change
     would redeploy the website, and the two repositories' tool chains would
     be coupled.
3. **One playbook.** `doc/local-playbook.yml` builds the working tree into
   `build/doc/`.
   - Its content source is this repository (`url: ..`, `start_path: doc`,
     `branches: HEAD`). Antora reads the worktree of the checked-out branch,
     so local builds include uncommitted edits, and CI builds the commit it
     checked out.
   - CI and `check.sh` both build with it through `scripts/build_docs.sh`.
     The build runs with `--log-failure-level=warn`, so a broken xref, include
     or attribute fails it.
4. **CI and triggers** (`.github/workflows/docs.yml`).
   - Pull requests touching `doc/**`, `scripts/build_docs.sh` or the workflow
     run the docs build only.
   - Pushes to `main` that touch the same paths build and deploy, and so does
     `workflow_dispatch`.
   - Only the deploy job gets `pages: write` and `id-token: write`;
     everything else is `contents: read`.
   - CI runs only the docs build (owner, question 5). The doc examples run as
     tests in `check.sh`, which needs the EDG and Silicon toolchain. CI for the
     rest of the repository is follow-up work. If macOS runners cannot
     reproduce the toolchain-tied goldens, the owner's fallback is x64 in
     Docker.
5. **UI.**
   - The site uses the default Antora UI plus supplemental files in
     `doc/ui/supplemental/`: the logo lockup, the Ink/Sand palette, Archivo
     and the favicons. They are taken unchanged from the brand kit and the
     website.
   - The bundle is a vendored zip in `doc/ui/`. Its SHA-256 is recorded in
     0001 and in `doc/ui/README.md`, and `scripts/build_docs.sh` checks it
     before every build. The default UI has no versioned release URL (its
     published URL follows `HEAD`) and no npm package, so vendoring is the
     only pin that cannot move.
   - The Boost UI bundle is not used.
   - BRAND.md's contrast table applies: Sand is never text on a light
     ground.
6. **Search.** `@antora/lunr-extension`, pinned exactly with the Antora
   packages in `doc/package.json` and `doc/package-lock.json`.
7. **What counts as a code example** (invariant 8).
   - **What counts.** A code example is a source block in a language cerastes
     reads: C, C++ or Viper. Each one is an include of a file under
     `doc/modules/ROOT/examples/`, through a tagged region, so the page never
     shows the expectation header.
   - **Example files are ordinary tests.** They have `// expect:` headers,
     and `scripts/run_tests.py` discovers them by default alongside `tests/`.
   - **Lint.** `scripts/check_doc_examples.py` rejects an inline C/C++/Viper
     block, an untagged include, an include of a file that is not a valid
     test, and an example no page uses.
   - **Shell command listings** (`scripts/bootstrap.sh`, …) are not examples.
     They must name commands and options that exist, and review checks them,
     as it checks every claim on a page.
8. **Pages describe only what exists.** There are no pages for future
   features, and no versioned docs until the first tagged release
   (`version: ~`).

## Consequences

- **Address.** The address in plan.md and the M1.5 acceptance
  (`https://cerastes.dev/doc/`) is replaced by `https://cerastes.dev/cerastes/`
  until the owner fixes the final address.
- **Owner steps that remain.** Of plan.md's four:
  - the `doc` repo and the dispatch token are no longer needed;
  - enabling Pages is done;
  - the "Documentation" link on cerastes.dev remains.
- **Deploys need a push.** Sessions do not push (owner, question 7). A docs
  change reaches the site when the owner pushes `main`, with no other manual
  step.
- **New toolchain parts.**
  - `scripts/bootstrap.sh` installs Node and runs `npm ci` in `doc/`.
  - `check.sh` gains a docs step after the tests.
  - The Node, Antora, lunr and UI-bundle pins are recorded in 0001.
