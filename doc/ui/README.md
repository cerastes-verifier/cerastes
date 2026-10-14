# Documentation UI

The user documentation (`doc/`, Antora) uses the Antora default UI, vendored as
a zip, plus a small supplemental UI that gives it the cerastes branding.
Decision record: `internal/decisions/0004-docs-site.md` (item 5).

## Vendored bundle

| | |
|---|---|
| File | `antora-ui-default.zip` |
| Source URL | https://gitlab.com/antora/antora-ui-default/-/jobs/artifacts/HEAD/raw/build/ui-bundle.zip?job=bundle-stable (redirects to `.../-/jobs/15385706035/artifacts/raw/build/ui-bundle.zip`) |
| Built from | antora-ui-default `master` at `0e38223adfd81eb74d4b1779e158f8ad05ff8923` ("make checkbox 1em instead of 1rem", 2026-07-16) |
| Downloaded | 2026-10-14 |
| SHA-256 | `e0ac81aad26961a9cd3cf9bce692a598eff4a1bd2822ce73c9cdc13a15a1e70d` |

How the commit was established: the download redirects to GitLab job 15385706035.
The GitLab API (`/projects/antora%2Fantora-ui-default/pipelines/2682652257/jobs`)
lists that job as `bundle-stable` of pipeline 2682652257, whose `sha` is
`0e38223a...`, the head of `master` on the download date; the zip's entries
are dated 2026-07-16 17:58, the job's finish time. The default UI has no
versioned release URL and no npm package, so only the vendored copy is ever
used; `scripts/build_docs.sh` refuses to build if the zip's SHA-256 differs
from the constant `UI_BUNDLE_SHA256` in that script. To update the bundle,
replace the zip, change the constant, this file, and the pin in
`internal/decisions/0001-toolchain-pins.md` together.

## Supplemental files (`supplemental/`)

Override the bundle's files of the same path.

| File | What |
|---|---|
| `partials/header-content.hbs` | Ink navbar with the lockup as the home link; keeps the lunr search box and the burger button; drops the default placeholder menu. Derived from the bundle's partial. |
| `partials/head-icons.hbs` | Brand favicons and `theme-color #161C2E`. |
| `partials/head-styles.hbs` | Archivo from Google Fonts (the website's URL), `site.css`, then `cerastes.css`. |
| `partials/footer-content.hbs` | cerastes, license, repository link. |
| `css/cerastes.css` | BRAND.md tokens and the overrides. Sand is never text on a light ground (BRAND.md contrast table); links use `--cerastes-sand-ink` (`#7f5f2d`, Sand scaled to about 59%, 5.9:1 on white), a docs-only token that is not in the brand kit. |

Code blocks keep the default UI's Roboto Mono; BRAND.md names Archivo as the
only typeface for brand text, and has no monospace.

## Copied assets (`supplemental/img/`), byte-identical

Sources: the website repository (`~/Documents/cerastes/cerastes-verifier.github.io`,
commit `b66fe767f2b89108b1a61f805265abc0ff73ef7c`, 2026-10-12) and the brand kit
(`~/Documents/cerastes/brand/cerastes-brand-kit.zip`). Each website file was
checked to be identical to the kit's copy.

| File | Copied from | SHA-256 |
|---|---|---|
| `cerastes-lockup-on-dark.svg` | website `src/assets/cerastes-lockup-on-dark.svg` (= kit `logo/`) | `6815f3a0664ef88e5f1fb2d1876bd6ad34fbed861b73a29166519d978e2e7fb1` |
| `favicon.ico` | website `public/favicon.ico` (= kit `web/`) | `3e451ec1b1f033171a94083df2bcdf50d8f8e9b8ad4f72ff3e88ecf959fbf81b` |
| `favicon.svg` | website `public/favicon.svg` (= kit `web/`) | `34cbe30319de898494d43140169b355196ed917e103f7fd5ff239b6468da805e` |
| `apple-touch-icon.png` | website `public/apple-touch-icon.png` (= kit `web/`) | `96527ca81684e3ff23bd947b8f24b136e4a0c0632ec5492febb36d22df182233` |

Note: the loose `brand/cerastes-lockup-on-dark.svg` next to the kit zip has a
different SHA-256 (`63a313f7...`) from the kit's and the website's copy; the
website's and the kit's copy are used.
