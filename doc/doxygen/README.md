# Doxygen setup

Files used to build the HTML documentation:

| File | Purpose |
|------|---------|
| `../../Doxyfile` | Doxygen configuration (tested with Doxygen 1.12.0) |
| `header.html` | Page header; loads the theme extensions (dark-mode toggle, copy button, heading links, interactive table of contents) |
| `custom.css` | DisCostiC colours on top of the theme |
| `md_filter.py` | Adapts the GitHub Markdown guides for Doxygen while building (anchors, links, emoji) |
| `../doxygen-awesome/` | [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) v2.4.2, MIT licence |

Build locally from the repository root:

```bash
doxygen Doxyfile            # needs doxygen and graphviz (dot)
xdg-open doc/html/index.html
```

Warnings go to `doc/doxygen/doxygen-warnings.log` (not committed). After building, commit `doc/html/` together with the source changes; GitHub Pages serves it from the `main` branch.

To update the theme, copy the same files from a newer doxygen-awesome-css release into `../doxygen-awesome/`.
If Doxygen is upgraded, regenerate `header.html` with `doxygen -w html header.html footer.html style.css Doxyfile` and copy the `<script>` block before `</head>` across.
