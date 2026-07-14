import unittest
import os
import sys
import tempfile
import subprocess

_DLL_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))

_LOADER_SCRIPT = """\
import ctypes, sys, os
dll = os.path.join(r"{dll_dir}", "slider.dll")
lib = ctypes.CDLL(dll)
lib.slider_load.restype = ctypes.c_void_p
lib.slider_load.argtypes = [ctypes.c_char_p]
lib.slider_free.argtypes = [ctypes.c_void_p]
path = r"{path}"
s = lib.slider_load(path.encode("utf-8"))
if s:
    lib.slider_free(s)
"""


def load_and_capture_stderr(path):
    """Load a markdown file in a subprocess and capture C library stderr output."""
    script = _LOADER_SCRIPT.format(dll_dir=_DLL_DIR, path=path.replace("\\", "\\\\"))
    f = tempfile.NamedTemporaryFile(mode="w", suffix=".py", delete=False,
                                    dir=tempfile.gettempdir())
    f.write(script)
    f.close()
    try:
        result = subprocess.run(
            [sys.executable, f.name],
            capture_output=True, text=True, timeout=10
        )
        return result.stderr
    finally:
        os.unlink(f.name)


class TestEmptyWarnings(unittest.TestCase):
    def _tmp_md(self, content):
        f = tempfile.NamedTemporaryFile(mode="w", suffix=".md", delete=False,
                                        encoding="utf-8", dir=tempfile.gettempdir())
        f.write(content)
        f.close()
        return f.name

    def test_empty_file_warning(self):
        path = self._tmp_md("")
        try:
            err = load_and_capture_stderr(path)
            self.assertIn("vacio", err)
        finally:
            os.unlink(path)

    def test_spaces_only_warning(self):
        path = self._tmp_md("   \n   \n")
        try:
            err = load_and_capture_stderr(path)
            self.assertIn("sin contenido", err)
        finally:
            os.unlink(path)

    def test_tabs_only_warning(self):
        path = self._tmp_md("\t\t\n\t\n")
        try:
            err = load_and_capture_stderr(path)
            self.assertIn("sin contenido", err)
        finally:
            os.unlink(path)

    def test_newlines_only_warning(self):
        path = self._tmp_md("\n\n\n\n")
        try:
            err = load_and_capture_stderr(path)
            self.assertIn("sin contenido", err)
        finally:
            os.unlink(path)

    def test_only_separators_warning(self):
        path = self._tmp_md("---\n---\n---\n")
        try:
            err = load_and_capture_stderr(path)
            self.assertIn("sin contenido", err)
        finally:
            os.unlink(path)

    def test_content_no_warning(self):
        path = self._tmp_md("# Title\n\nBody text\n")
        try:
            err = load_and_capture_stderr(path)
            self.assertNotIn("LINTER WARNING", err)
        finally:
            os.unlink(path)

    def test_single_line_no_warning(self):
        path = self._tmp_md("Hello\n")
        try:
            err = load_and_capture_stderr(path)
            self.assertNotIn("LINTER WARNING", err)
        finally:
            os.unlink(path)


if __name__ == "__main__":
    unittest.main()
