# Contributing to C-Slides

Thank you for your interest in contributing to **C-Slides**! We welcome contributions from everyone, whether you are fixing a bug, adding a new visual theme, optimizing Cairo/Pango rendering performance, or expanding hardware/platform backends.

This document provides guidelines and instructions for contributing to the C-Slides codebase.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
  - [Reporting Bugs](#reporting-bugs)
  - [Suggesting Features](#suggesting-features)
  - [Submitting Pull Requests](#submitting-pull-requests)
- [Development Workflow](#development-workflow)
  - [Prerequisites](#prerequisites)
  - [Building from Source](#building-from-source)
  - [Running Tests](#running-tests)
- [Project Architecture](#project-architecture)
- [C Coding Conventions](#c-coding-conventions)
- [Git Commit Guidelines](#git-commit-guidelines)
- [License](#license)

---

## Code of Conduct

Please treat all members of the community with respect, kindness, and professionalism. We aim to keep C-Slides an inclusive, constructive, and collaborative environment.

---

## How Can I Contribute?

### Reporting Bugs

Before opening a bug report, please check existing GitHub issues to verify that the bug hasn't already been reported.

When submitting a bug report, please include:
1. **OS & Environment**: Linux distribution (X11 / Wayland details) or Windows version.
2. **Dependency Versions**: Cairo and Pango versions installed.
3. **Reproduction Steps**: Minimal Markdown sample file (`.md`) that reproduces the rendering or behavior issue.
4. **Expected vs. Actual Behavior**: Description and, if applicable, screenshots or logs.

### Suggesting Features

Feature requests are always welcome! When proposing a feature:
- Explain the specific problem or use case it solves.
- Detail proposed CLI flags, frontmatter keys, or markdown directives (e.g., `<!-- img: ... -->`).
- Mention any Cairo/Pango layout or backend performance implications.

### Submitting Pull Requests

1. **Fork the Repository** on GitHub.
2. **Create a Feature Branch**: `git checkout -b feat/my-new-feature` or `fix/issue-description`.
3. **Implement Changes**: Adhere to the project's C code conventions and ensure zero memory leaks.
4. **Run the Test Suite**: Confirm all unit and integration tests pass (`make test`).
5. **Commit & Push**: Follow structured commit messages (see [Git Commit Guidelines](#git-commit-guidelines)).
6. **Open a Pull Request**: Provide a clear description of the problem solved and key architectural changes made.

---

## Development Workflow

### Prerequisites

#### Linux (Debian / Ubuntu / Arch / Fedora)
```bash
# Ubuntu / Debian
sudo apt update
sudo apt install build-essential libcairo2-dev libpango1.0-dev libx11-dev python3
```

#### Windows (MSYS2 MinGW64)
Run from the MSYS2 MinGW64 terminal (not standard PowerShell):
```bash
pacman -S mingw-w64-x86_64-cairo mingw-w64-x86_64-pango mingw-w64-x86_64-gcc make python3
```

### Building from Source

To compile the native executable (`slides`) and the shared library (`libslider.so` / `slider.dll`):

```bash
make
```

To clean build artifacts:
```bash
make clean
```

### Running Tests

C-Slides uses a Python test harness interacting directly with `libslider` via `ctypes`:

```bash
make test
```

All 600+ unit and integration tests must pass before merging PRs. If you add new parser features or exporter functionality, please add corresponding test cases in `tests/`.

---

## Project Architecture

Understanding where components reside will help guide your contributions:

```
slider.h              Public C API header
src/
  core/
    parser.c          Markdown parser, frontmatter extraction, transitions
    themes.c          11 built-in color palettes & frontmatter custom overrides
    highlighter.c     Syntax highlighting token parser
    internal.h        Internal structures (Slide, Slider, Theme)
  render/
    renderer.c        Main Cairo/Pango rendering pipeline
    render_util.c     Utilities and 64-slot image cache engine
    render_table.c    Proportional table grid calculation & rendering
    render_code.c     Code block syntax highlighters
    render_transition.c Transition interpolation engines (easing/smoothstep)
    export_*.c        PNG, JPG, PDF, GIF, and SVG exporter backends
  ui/
    backend_win32.c   Native Win32 double-buffering & GDI window handler
    backend_x11.c     Native X11 window backend & adaptive frame rate manager
    help.c            CLI manual page generator and usage details
    main.c            CLI parsing and application entry point
```

---

## C Coding Conventions

To ensure C-Slides remains lightweight, robust, and portable:

1. **Language Standard & Compiler Warnings**:
   - Code must target standard C99/C11 and compile cleanly without warnings (`-Wall -Wextra -Wpedantic`).
   - Maintain strict cross-platform compatibility across Win32 and X11/Linux.

2. **Memory Management**:
   - Always check dynamic allocations (`malloc`, `calloc`, `realloc`).
   - Clean up all Cairo surfaces (`cairo_surface_destroy`), contexts (`cairo_destroy`), and Pango layouts (`g_object_unref`).
   - Ensure zero memory leaks. Test with `valgrind` or standard AddressSanitizer (`-fsanitize=address`) on Linux.

3. **Naming Conventions**:
   - Functions and variables: `snake_case` (e.g., `slider_render_slide`, `image_cache_get`).
   - Types and structs: `PascalCase` or `snake_case_t` (e.g., `SliderContext`, `slide_theme_t`).
   - Preprocessor macros and constants: `ALL_CAPS` (e.g., `MAX_CACHE_SLOTS`, `DEFAULT_FPS`).

4. **Modular Design**:
   - Keep functions modular and focused on a single responsibility.
   - Encapsulate rendering logic inside `src/render/` without leaking platform window specifics into core parsers.

---

## Git Commit Guidelines

Write clear and descriptive commit messages following standard conventions:

- `feat(render): add support for rounded image borders`
- `fix(parser): resolve blockquote trailing newline parsing issue`
- `docs(readme): add Arch Linux installation instructions`
- `refactor(x11): optimize redraw rate during idle state`

---

## License

By contributing to **C-Slides**, you agree that your contributions will be licensed under the project's [MIT License](LICENSE).
