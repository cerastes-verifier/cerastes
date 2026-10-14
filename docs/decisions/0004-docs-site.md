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
  `301 → https://cerastes.dev/cerastes/`. That much is GitHub's default; what
  appears below `/cerastes/` is whatever the deployed Pages artifact
  contains.
- On 2026-10-13 `https://cerastes.dev` served GitHub's `*.github.io`
  certificate: `www` was proxied by Cloudflare, which blocks GitHub's
  certificate issuance. The owner fixed the DNS and enabled "Enforce HTTPS"
  on 2026-10-14, and the domain now has its own Let's Encrypt certificate.

## Decision

1. **Hosting.** The docs are part of this repository's GitHub Pages project
   site, deployed by GitHub Actions from this repository, at
   **`https://cerastes.dev/cerastes/doc/`**.
   - **Org convention** (owner, 2026-10-14). Every repository of the
     organization serves its user docs at `https://cerastes.dev/<repo>/doc/`.
     GitHub supplies `/<repo>/`, which is where it serves the repository's
     project site under the org's custom domain. The `/doc/` part is a layout
     each repository builds into its Pages artifact. Here, the workflow
     stages the Antora output at `doc/` in the artifact, and adds a root
     `index.html` that redirects `/cerastes/` to `doc/`.
   - **Where the address is set.** `site.url` is set in one place,
     `doc/local-playbook.yml`. Local builds still go to `build/doc/`. Antora
     writes relative links in ordinary pages, so they work under any prefix.
     `site.url` feeds the sitemap, the canonical links and `404.html`, whose
     links are root-absolute (`/cerastes/doc/_/…`). GitHub Pages serves a
     custom 404 page only from the site root, so the workflow also copies
     `404.html` to the artifact root, where those links still resolve.
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
   - Pull requests touching `doc/**`, `scripts/build_docs.sh`,
     `scripts/check_doc_examples.py`, `scripts/run_tests.py` (which the lint
     imports) or the workflow run the source lint, the docs build and the
     rendered-HTML lint (item 7), but do not deploy.
   - Pushes to `main` that touch the same paths do the same, then deploy, and
     so does `workflow_dispatch`.
   - Only the deploy job gets `pages: write` and `id-token: write`;
     everything else is `contents: read`, and checkout does not keep its
     credentials.
   - Actions are pinned to full commit SHAs, with the release in a comment,
     and the runner is `ubuntu-24.04`. `npm ci` runs with `--ignore-scripts`.
   - Concurrency is set for the whole workflow. Runs that deploy (pushes and
     dispatches on `main`) share one group and do not cancel a running deploy.
     GitHub keeps at most one pending run per group, so a newer push replaces
     an older pending one, and deploys land in order with the newest last. All
     other runs cancel their own superseded runs and never queue behind a
     deploy.
   - CI runs only the docs: the lints and the build (owner, question 5). The
     doc examples run as tests in `check.sh`, which needs the EDG and Silicon
     toolchain. CI for the
     rest of the repository is follow-up work. If macOS runners cannot
     reproduce the toolchain-tied goldens, the owner's fallback is x64 in
     Docker.
5. **UI.**
   - The site uses the default Antora UI plus supplemental files in
     `doc/ui/supplemental/`: the logo lockup, the Ink/Sand palette, Archivo
     and the favicons. They are taken unchanged from the brand kit and the
     website.
   - The bundle is a vendored zip in `doc/ui/`. Its SHA-256 is recorded in
     0001 and in `doc/ui/README.md`. `scripts/build_docs.sh` checks it before
     every build and passes it to Antora with `--ui-bundle-url`, so a
     playbook cannot substitute another bundle. The default UI has no versioned release URL (its
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
   - **Two checks, in `check.sh` and in CI.**
     - *Source lint.* `scripts/check_doc_examples.py` reads the pages. A
       C/C++/Viper block may hold exactly one line,
       `include::example$<test>[tag(s)=…]`. It rejects, among others
       (`tests/README.md` has the full list):
       - inline code, `partial$` and attachment includes, and untagged
         includes;
       - tags that are not plain `// tag::x[]` line comments, or that sit in
         dead code (`#if 0`, comments, raw strings, macro arguments, `\`
         continuations);
       - an example that no page uses, and any file under `examples/`
         that is not a test or a golden;
       - C/C++/Viper source anywhere else under `doc/modules/`;
       - a module other than `ROOT`;
       - a block whose language is undeclared or not allowed, and a
         component-wide `source-language`;
       - literal and verse blocks, indented paragraphs, `l`/`m` table cells,
         passthrough blocks, raw HTML in `pass:[…]`, and `subs=` overrides.
       It is a best-effort reader, not an AsciiDoc parser.
     - *Rendered check (the backstop).* After the build,
       `scripts/check_doc_examples.py --html build/doc` classifies every
       preformatted block (`<pre>`) in the built pages:
       - a block in a cerastes language (C, C++, Viper) must equal a tagged
         region of a test under `examples/`;
       - a `sh`, `console` or `text` block is allowed;
       - anything else is an error: no language, another language, or raw
         HTML.

       It also fails if the site has fewer cerastes-language blocks than the
       pages declare, and on rendered text that shows broken inline markup.
       Together with the source lint's ban on raw HTML (`++++`,
       `pass:[<…]`, `subs=`), this means that code the reader sees in a block
       is code the harness runs, unless it is written into an explicitly
       non-code listing. That case is left to review (below), as is raw HTML
       that evades both checks, which needs deliberate effort that review
       would see.
     - *Inline markup.* Some broken markup leaves well-formed HTML (a
       `libc++` pair that swallows text, a stray `**` pair), so the source
       lint is the only guard for it. Any `++` in prose must be inside
       `pass:[…]` (C++ is written `pass:[C++]`). A doubled `__`, `**` or `##`
       in one paragraph must be inside a literal span (`+…+`). Only the
       languages above may be declared on a block.
     - *What review still owns.* Content of a `text` block (C++ written there
       is not tested), and inline code spans in prose.
   - **Shell command listings** (`scripts/bootstrap.sh`, …) are not examples.
     They must name commands and options that exist, and review checks them,
     as it checks every claim on a page.
8. **Pages describe only what exists.** There are no pages for future
   features, and no versioned docs until the first tagged release
   (`version: ~`).

## Consequences

- **Address.** The address in plan.md and the M1.5 acceptance,
  `https://cerastes.dev/doc/`, becomes `https://cerastes.dev/cerastes/doc/`.
  plan.md is updated to match.
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
