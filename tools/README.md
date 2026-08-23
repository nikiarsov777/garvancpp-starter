# tools/

One-shot migration helpers used to bootstrap the popup-style i18n layout
(see `/garvan/i18n` in the running docs or the "Internationalisation
(i18n)" section of the top-level `README.md`).

| Script | What it does |
| --- | --- |
| `extract_docs_i18n.py` | Parses the hardcoded `strings()` and `pages()` initializer lists in `routes/DocsRouter.cpp` and emits one flat JSON dictionary per language in `public/langs/`. Ran once during the migration from a compiled-in dict to on-disk JSON. |
| `extract_pages_i18n.py` | Walks every `public/pages/<lang>/**/*.html`, extracts text-carrying elements (`h1..h6`, `p`, `li`, `td`, `th`, `blockquote`, `dt`, `dd`) into `t_page_<slug>_e###` keys inside the per-lang JSON dicts, and deploys a single canonical template (with `{{{t_...}}}` placeholders) that replaces every per-lang HTML copy. |

Both scripts are safe to re-run — they guard against re-extracting from
an already-templated file (that would corrupt the dicts with placeholder
strings). Day-to-day maintenance no longer needs them: add or edit
translations directly in `public/langs/<lang>.json`.

Dependencies: Python 3.10+, `beautifulsoup4` (`pip install beautifulsoup4 lxml`).
