#!/usr/bin/env python3
"""Doxygen input filter for the Markdown guides (README.md and doc/*.md).

The guides are written for GitHub. This filter adapts them for the Doxygen
site on the fly, without changing the files in the repository:

* <a name="x"></a>               -> \\anchor x  (so [text](#x) links resolve)
* links to other guides          -> links to their Doxygen page or anchor
* links to code/config files     -> links to the file on GitHub
* GitHub emoji shortcodes        -> Unicode emoji
* **Label** directly before emoji -> space added so the bold renders
* @Name                           -> escaped, so Doxygen does not treat it as a command
* each doc/*.md page gets a title and a stable page label

Doxygen calls it as:  python3 doc/doxygen/md_filter.py <file>
"""
import os
import re
import sys

REPO_URL = "https://github.com/RRZE-HPC/DisCostiC-Sim/blob/main/"

EMOJI = {
    "octocat": "🐙", "signal_strength": "📶", "thought_balloon": "💭",
    "speech_balloon": "💬", "green_circle": "🟢", "warning": "⚠️",
}

path = sys.argv[1]
root = os.getcwd()  # Doxygen runs from the repository root
rel_file = os.path.relpath(os.path.abspath(path), root)
file_dir = os.path.dirname(rel_file)
text = open(path, encoding="utf-8").read()


def page_label(md_path):
    """Stable Doxygen page label for a guide, e.g. doc/README_MODEL.md -> readme_model."""
    return os.path.splitext(os.path.basename(md_path))[0].lower()


def fix_link(match):
    label, target = match.group(1), match.group(2)
    if re.match(r"^[a-z]+:|^#|^mailto:", target):          # absolute URL or same-page anchor
        return match.group(0)
    if "@" in target and "/" not in target:                   # bare e-mail address
        return f"[{label}](mailto:{target})"
    target_path, _, anchor = target.partition("#")
    resolved = os.path.normpath(os.path.join(file_dir, target_path)).replace(os.sep, "/")
    if resolved.endswith(".md"):
        if anchor:                                            # anchors are global in Doxygen
            return f"[{label}](#{anchor})"
        if resolved == "README.md":
            return f"[{label}](index.html)"
        return f"[{label}](#{page_label(resolved)})"
    return f"[{label}]({REPO_URL}{resolved}{'#' + anchor if anchor else ''})"


# 1. HTML anchors used by the GitHub links -> Doxygen anchors
text = re.sub(r'<a name="([^"]+)"></a>', r"\\anchor \1", text)
# 2. Relative links (skip images: ![...](...))
text = re.sub(r"(?<!!)\[([^\]]*)\]\(([^)\s]+)\)", fix_link, text)
# 3. "@Name" (GitHub handles, NHR@FAU) would be read as a Doxygen command
text = re.sub(r"@(?=[A-Z])", r"\\@", text)
# 4. Emoji shortcodes
text = re.sub(r":([a-z_]+):", lambda m: EMOJI.get(m.group(1), m.group(0)), text)
# 5. "**Label**💬": Doxygen only closes bold before a space or punctuation
text = re.sub(r"(\*\*[^*\n]+\*\*)(?=[^\s*.,;:!?)\]])", r"\1 ", text)

# 6. Title + label for the doc/ guides. The first "## " heading becomes the
#    page title and is removed from the body so it does not appear twice.
if rel_file.startswith("doc/") and not text.lstrip().startswith("# "):
    m = re.search(r"^## (.+)$", text, flags=re.M)
    title = m.group(1).strip() if m else page_label(rel_file)
    if m:
        text = text[:m.start()] + text[m.end():]
    text = f"# {title} {{#{page_label(rel_file)}}}\n" + text
elif rel_file.startswith("doc/"):
    text = re.sub(r"^# (.+)$", lambda m: f"# {m.group(1)} {{#{page_label(rel_file)}}}",
                  text, count=1, flags=re.M)

sys.stdout.write(text)
