#!/usr/bin/env python3
"""Extract per-page body prose from public/pages/{lang}/**/*.html into
translation keys stored in public/langs/{lang}.json, and emit canonical
HTML templates using {{{t_...}}} placeholders.

NOTE: This script targets the pre-migration layout with per-lang page
directories (public/pages/en/, public/pages/bg/, ...). After the popup-
style migration collapsed the tree to a single public/pages/ directory,
this script is no longer part of the day-to-day workflow — edit
public/langs/<lang>.json directly. Kept for git-blame / audit purposes.

Guards against re-extracting from an already-templated file (would
corrupt dicts with placeholder strings).

Usage:
    python3 tools/extract_pages_i18n.py            # all pages
    python3 tools/extract_pages_i18n.py garvan/orm # single page
"""
from __future__ import annotations

import json
import sys
from difflib import SequenceMatcher
from pathlib import Path

from bs4 import BeautifulSoup, NavigableString, Tag

ROOT       = Path(__file__).resolve().parent.parent
PAGES_ROOT = ROOT / "public" / "pages"
LANGS_ROOT = ROOT / "public" / "langs"

LANGS = ("en", "bg", "ru", "es", "tr", "pt")

TEXT_TAGS = {"h1","h2","h3","h4","h5","h6",
             "p","li","td","th","blockquote","dt","dd","summary","figcaption"}
SKIP_TAGS = {"pre","script","style","svg","picture","source"}


def slug_page_key(page_key: str) -> str:
    return page_key.replace("/", "_").replace("-", "_")


def load_lang_dict(lang: str) -> dict:
    p = LANGS_ROOT / f"{lang}.json"
    return json.loads(p.read_text(encoding="utf-8")) if p.exists() else {}


def save_lang_dict(lang: str, d: dict):
    p = LANGS_ROOT / f"{lang}.json"
    p.write_text(json.dumps(dict(sorted(d.items())), ensure_ascii=False, indent=2) + "\n",
                 encoding="utf-8")


def inner_html(el: Tag) -> str:
    return "".join(str(c) for c in el.children).strip()


def collect_text_elements(soup: BeautifulSoup) -> list[Tag]:
    out = []
    for el in soup.find_all(True):
        if el.name in SKIP_TAGS:
            continue
        if any(p.name in SKIP_TAGS for p in el.parents if isinstance(p, Tag)):
            continue
        if el.name in TEXT_TAGS and el.get_text(strip=True):
            out.append(el)
    return out


def extract_page(page_key: str, all_dicts: dict[str, dict]) -> bool:
    en_path = PAGES_ROOT / "en" / (page_key + ".html")
    if not en_path.exists():
        # Post-migration: single canonical at pages/<key>.html
        en_path = PAGES_ROOT / (page_key + ".html")
    if not en_path.exists():
        print(f"  [skip] {page_key}: EN missing")
        return False

    en_raw = en_path.read_text(encoding="utf-8")
    if "{{{t_page_" in en_raw or "{{t_page_" in en_raw:
        print(f"  [skip] {page_key}: EN already templated")
        return False

    en_soup  = BeautifulSoup(en_raw, "lxml")
    en_elems = collect_text_elements(en_soup)
    slug = slug_page_key(page_key)
    keys = [f"t_page_{slug}_e{i:03d}" for i in range(1, len(en_elems) + 1)]

    for k, el in zip(keys, en_elems):
        all_dicts["en"][k] = inner_html(el)

    en_tags = [el.name for el in en_elems]
    for lang in LANGS:
        if lang == "en":
            continue
        lp = PAGES_ROOT / lang / (page_key + ".html")
        if not lp.exists():
            continue
        lp_raw = lp.read_text(encoding="utf-8")
        if "{{{t_page_" in lp_raw or "{{t_page_" in lp_raw:
            print(f"  [skip-lang] {lang}/{page_key}: already templated")
            continue
        soup = BeautifulSoup(lp_raw, "lxml")
        elems = collect_text_elements(soup)

        sm = SequenceMatcher(a=en_tags, b=[el.name for el in elems], autojunk=False)
        for op, i1, i2, j1, j2 in sm.get_opcodes():
            if op == "equal":
                for off in range(i2 - i1):
                    all_dicts[lang][keys[i1 + off]] = inner_html(elems[j1 + off])

    # Deploy canonical template
    tpl_soup = BeautifulSoup(en_raw, "lxml")
    tpl_elems = collect_text_elements(tpl_soup)
    for k, el in zip(keys, tpl_elems):
        el.clear()
        el.append(NavigableString("{{{" + k + "}}}"))
    body = tpl_soup.body
    tpl_html = "".join(str(c) for c in body.children) if body else str(tpl_soup)

    # Post-migration: single-file deploy. Pre-migration: per-lang.
    canonical = PAGES_ROOT / (page_key + ".html")
    if canonical.exists() or not (PAGES_ROOT / "en").exists():
        canonical.parent.mkdir(parents=True, exist_ok=True)
        canonical.write_text(tpl_html, encoding="utf-8")
    else:
        for lang in LANGS:
            target = PAGES_ROOT / lang / (page_key + ".html")
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(tpl_html, encoding="utf-8")

    print(f"  ok: {page_key} ({len(keys)} keys)")
    return True


def all_pages() -> list[str]:
    root = PAGES_ROOT / "en" if (PAGES_ROOT / "en").exists() else PAGES_ROOT
    return [str(p.relative_to(root).with_suffix(""))
            for p in sorted(root.rglob("*.html"))]


def main():
    filter_key = sys.argv[1] if len(sys.argv) > 1 else None
    pages = [filter_key] if filter_key else all_pages()
    all_dicts = {l: load_lang_dict(l) for l in LANGS}
    ok = sum(1 for p in pages if extract_page(p, all_dicts))
    for l in LANGS:
        save_lang_dict(l, all_dicts[l])
    print(f"\n{ok}/{len(pages)} pages extracted")
    print("sizes:", {l: len(all_dicts[l]) for l in LANGS})


if __name__ == "__main__":
    main()
