---
name: bump-version
description: Use when releasing or bumping the vista-sdk version across all SDKs (C#, JS, Python) - updates every source-of-truth version location, toggles the CI run-number revision between release and preview, and verifies the result
---

# Bump version across all SDKs

The vista-sdk version lives in a handful of **source-of-truth locations**. CI
appends a run-number "revision" at publish time for preview builds. Keeping all
of this in sync by hand is error-prone — use the script.

```bash
.github/skills/bump-version/bump-version.sh <command>
```

## Two modes (the target is derived from the current version)

| Mode | Command | From → To | npm dist-tag | CI run-number |
|------|---------|-----------|--------------|---------------|
| **release** | `release` | `X.Y.Z-preview` → `X.Y.Z` | `latest` | **removed** |
| **preview** | `preview` | `X.Y.Z` → `X.Y.(Z+1)-preview` | `preview` | **re-introduced** |

You normally don't type a version — it's derived from the current state, and
re-running in the same mode is a **no-op**:

```bash
bump-version.sh --current        # show current version + which CI mode is active
bump-version.sh release          # finalize current preview:  0.3.1-preview -> 0.3.1
bump-version.sh preview          # open next preview:          0.3.1 -> 0.3.2-preview
bump-version.sh preview          # already a preview:          0.3.2-preview -> no change

bump-version.sh preview minor    # bump minor instead of patch: 0.3.1 -> 0.4.0-preview
bump-version.sh preview major    # bump major:                  0.3.1 -> 1.0.0-preview
bump-version.sh release 0.5.0    # explicit release version (override)
bump-version.sh preview 0.5.0    # explicit preview version  -> 0.5.0-preview
```

- `release` with no argument drops the `-preview` suffix from the current
  version (the patch number is **not** changed — the increment happens when you
  open the next preview). Already-released → no change.
- `preview` with no argument increments the **patch** of the current core and
  adds `-preview`. Already-preview → no change. Use `minor`/`major` to bump a
  different level, or pass an explicit `X.Y.Z` to override entirely.

## What it changes

**Version strings (all four source-of-truth locations):**

| SDK | File | Field |
|-----|------|-------|
| JS (core) | `js/packages/vista-sdk/package.json` | `version` |
| JS (experimental) | `js/packages/vista-sdk-experimental/package.json` | `version` + `dnv-vista-sdk` peerDep floor `>=X.Y.Z-0` |
| Python | `.github/workflows/build-python.yml` | `BASE_VERSION` |
| C# | `.github/workflows/build-csharp.yml` | `/p:Version=...` |

**CI revision (the run-number suffix), toggled by mode:**

| SDK | Preview form | Release form |
|-----|--------------|--------------|
| C# | `/p:Version=<v>-${{ steps.rev.outputs.count }}` | `/p:Version=<v>` |
| Python | `VERSION="${BASE_VERSION}-${REV}"` | `VERSION="${BASE_VERSION}"` |
| JS | `npm version <v>.${{ steps.rev.outputs.count }}` … `--tag preview` | `npm version <v> --allow-same-version` … `--tag latest` |

The revision is **removed for a release** (so the published version is a clean
`X.Y.Z`) and **re-introduced on the next preview bump** — running
`preview` after a `release` restores all the revision logic automatically.

The script then verifies every change (`ok` / `FAIL` per item) and exits
non-zero if anything is off.

## Release / preview cycle

```bash
# 1. Cut the release (drops -preview from the current version)
.github/skills/bump-version/bump-version.sh release      # 0.3.1-preview -> 0.3.1
git commit -am "release: 0.3.1"
git tag -a v0.3.1 -m "Release 0.3.1"
git push && git push origin v0.3.1          # CI publishes clean 0.3.1 (latest)

# 2. Open the next preview (patch + -preview)
.github/skills/bump-version/bump-version.sh preview      # 0.3.1 -> 0.3.2-preview
git commit -am "chore: 0.3.2-preview"
git push                                      # preview publish is optional
```

When you start a preview, the script prints a **soft, non-blocking** check: if
the previous version was a release, it tells you whether a matching `vX.Y.Z` git
tag exists (a hint that the release was tagged/deployed). It never blocks.

## How versions stay unique

Preview builds publish `X.Y.Z-preview-<count>`, where `<count>` is
`git rev-list --count <latest-tag>..HEAD` — every commit on `main` gets a
distinct, increasing number, so no two builds collide (and the registries reject
duplicates as a backstop). Tagging a release (`vX.Y.Z`) resets that counter,
which is why a release bump and a tag go together.

## Notes

- Requires `node` (already a JS dev dependency) and `git`.
- Choosing **major / minor / patch** is a human SemVer call. `preview` defaults
  to a patch bump; pass `minor`/`major` (or an explicit version) when you mean
  something bigger. MAJOR = breaking, MINOR = feature, PATCH = fix.
- `python/src/vista_sdk/__init__.py` is **not** a source of truth — CI rewrites
  `__version__` from `BASE_VERSION` at build time.
- All edits preserve existing file formatting (JSON indentation/key order; only
  the relevant lines change) and are idempotent and reversible between modes.
