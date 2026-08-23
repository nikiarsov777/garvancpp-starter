#!/usr/bin/env python3
"""Extract i18n dict and PageMeta title/desc/toc from DocsRouter.cpp.

Emits public/langs/{en,bg,ru,es,tr,pt}.json — one flat JSON per language
with every translation key. Existing hand-written translations inside
the C++ initializer list are preserved verbatim.

Usage:
    python3 tools/extract_docs_i18n.py
"""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC  = ROOT / "routes" / "DocsRouter.cpp"
OUT  = ROOT / "public" / "langs"

LANGS = ("en", "bg", "ru", "es", "tr", "pt")

# {"t_key", { {"en","..."}, {"bg","..."}, ... }},
KEY_RE = re.compile(
    r'\{"(?P<key>t_[A-Za-z0-9_]+)"\s*,\s*\{(?P<body>.*?\})\}\}',
    re.DOTALL,
)
PAIR_RE = re.compile(
    r'\{"(?P<lang>[a-z]{2})"\s*,\s*"(?P<val>(?:[^"\\]|\\.)*)"\}',
    re.DOTALL,
)

# PageMeta entry — {"key", { "key","section", "title_en","title_bg",
#                            "desc_en","desc_bg", { <toc> }, "prev","next"}}
PAGE_HEADER_RE = re.compile(
    r'\{"(?P<key>[^"]+)"\s*,\s*\{\s*'
    r'"(?P<k2>[^"]+)"\s*,\s*"(?P<section>[^"]*)"\s*,\s*'
    r'"(?P<title_en>(?:[^"\\]|\\.)*)"\s*,\s*"(?P<title_bg>(?:[^"\\]|\\.)*)"\s*,\s*'
    r'"(?P<desc_en>(?:[^"\\]|\\.)*)"\s*,\s*"(?P<desc_bg>(?:[^"\\]|\\.)*)"\s*,\s*'
    r'\{(?P<toc>.*?)\}\s*,\s*'
    r'"(?P<prev>[^"]*)"\s*,\s*"(?P<next>[^"]*)"\s*\}\s*\}',
    re.DOTALL,
)
TOC_ITEM_RE = re.compile(
    r'\{\s*"(?P<anchor>[^"]*)"\s*,\s*'
    r'"(?P<en>(?:[^"\\]|\\.)*)"\s*,\s*'
    r'"(?P<bg>(?:[^"\\]|\\.)*)"\s*,\s*'
    r'(?P<sub>true|false)\s*\}',
    re.DOTALL,
)

def unescape_cxx(s: str) -> str:
    out, i = [], 0
    while i < len(s):
        c = s[i]
        if c == "\\" and i + 1 < len(s):
            m = {"n":"\n","t":"\t","r":"\r",'"':'"',"\\":"\\"}.get(s[i+1])
            if m is not None:
                out.append(m); i += 2; continue
        out.append(c); i += 1
    return "".join(out)


def parse_strings(text: str) -> dict[str, dict[str, str]]:
    d = {}
    for m in KEY_RE.finditer(text):
        d[m["key"]] = {p["lang"]: unescape_cxx(p["val"])
                       for p in PAIR_RE.finditer(m["body"])}
    return d


def parse_pages(text: str) -> dict[str, dict]:
    out = {}
    i = text.find("::pages()")
    if i >= 0:
        text = text[i:]
    for m in PAGE_HEADER_RE.finditer(text):
        toc = [{"anchor": t["anchor"],
                "en": unescape_cxx(t["en"]),
                "bg": unescape_cxx(t["bg"]),
                "sub": t["sub"] == "true"}
               for t in TOC_ITEM_RE.finditer(m["toc"])]
        out[m["key"]] = {
            "title_en": unescape_cxx(m["title_en"]),
            "title_bg": unescape_cxx(m["title_bg"]),
            "desc_en":  unescape_cxx(m["desc_en"]),
            "desc_bg":  unescape_cxx(m["desc_bg"]),
            "toc":      toc,
        }
    return out


def page_key_slug(key: str) -> str:
    return key.replace("/", "_").replace("-", "_")


def build_lang_dicts(strings, pages):
    lang = {l: {} for l in LANGS}
    for key, per in strings.items():
        for l in LANGS:
            if l in per:
                lang[l][key] = per[l]
    for pkey, pm in pages.items():
        slug = page_key_slug(pkey)
        lang["en"][f"t_page_{slug}_title"] = pm["title_en"]
        lang["en"][f"t_page_{slug}_desc"]  = pm["desc_en"]
        if pm["title_bg"]:
            lang["bg"][f"t_page_{slug}_title"] = pm["title_bg"]
        if pm["desc_bg"]:
            lang["bg"][f"t_page_{slug}_desc"]  = pm["desc_bg"]
        for item in pm["toc"]:
            anchor_slug = item["anchor"].replace("-", "_")
            k = f"t_toc_{slug}_{anchor_slug}"
            lang["en"][k] = item["en"]
            if item["bg"]:
                lang["bg"][k] = item["bg"]
    return lang


def main():
    text = SRC.read_text(encoding="utf-8")
    strings = parse_strings(text)
    pages   = parse_pages(text)
    print(f"strings: {len(strings)} keys, pages: {len(pages)} entries")

    dicts = build_lang_dicts(strings, pages)
    OUT.mkdir(parents=True, exist_ok=True)
    for l in LANGS:
        p = OUT / f"{l}.json"
        # Merge with existing to preserve any body-prose keys already there.
        existing = json.loads(p.read_text(encoding="utf-8")) if p.exists() else {}
        existing.update(dicts[l])
        p.write_text(json.dumps(dict(sorted(existing.items())),
                                ensure_ascii=False, indent=2) + "\n",
                     encoding="utf-8")
        print(f"  {l}.json: {len(existing)} keys")


if __name__ == "__main__":
    main()
