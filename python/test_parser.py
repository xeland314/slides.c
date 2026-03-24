import unittest
import os
from c_slides_bindings import CSlides, LineType

class TestParser(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "libslider.so"))
        cls.cs = CSlides(lib_path)

    def test_parse_title(self):
        sl = self.cs.parse_line("# Mi Titulo")
        self.assertEqual(sl.type, LineType.TITLE)
        self.assertEqual(sl.text.decode('utf-8'), "Mi Titulo")

    def test_parse_subtitle(self):
        sl = self.cs.parse_line("## Mi Subtitulo")
        self.assertEqual(sl.type, LineType.SUBTITLE)
        self.assertEqual(sl.text.decode('utf-8'), "Mi Subtitulo")

    def test_parse_blockquote(self):
        sl = self.cs.parse_line("> Nota importante")
        self.assertEqual(sl.type, LineType.BLOCKQUOTE)
        self.assertEqual(sl.text.decode('utf-8'), "Nota importante")

    def test_parse_bullets(self):
        # Nivel 1
        sl = self.cs.parse_line("- Item 1")
        self.assertEqual(sl.type, LineType.BULLET1)
        self.assertEqual(sl.text.decode('utf-8'), "Item 1")

        # Nivel 2
        sl = self.cs.parse_line("  - Item 2")
        self.assertEqual(sl.type, LineType.BULLET2)
        self.assertEqual(sl.text.decode('utf-8'), "Item 2")

    def test_parse_image(self):
        # Formato corto
        sl = self.cs.parse_line("!logo.png")
        self.assertEqual(sl.type, LineType.IMAGE)
        self.assertEqual(sl.text.decode('utf-8'), "logo.png")

        # Formato Markdown completo
        sl = self.cs.parse_line("![alt text](path/to/img.png)")
        self.assertEqual(sl.type, LineType.IMAGE)
        self.assertEqual(sl.text.decode('utf-8'), "path/to/img.png")

    def test_parse_table(self):
        # Fila normal
        sl = self.cs.parse_line("| A | B |")
        self.assertEqual(sl.type, LineType.TABLE_ROW)
        self.assertEqual(sl.ncols, 2)
        self.assertEqual(sl.cols[0].value.decode('utf-8'), "A")
        self.assertEqual(sl.cols[1].value.decode('utf-8'), "B")

        # Separador
        sl = self.cs.parse_line("|---|---|")
        self.assertEqual(sl.type, LineType.TABLE_SEP)

    def test_parse_task_list(self):
        # Unchecked
        sl = self.cs.parse_line("- [ ] Tarea pendiente")
        self.assertEqual(sl.type, LineType.TASK_UNCHECKED)
        self.assertEqual(sl.text.decode('utf-8'), "Tarea pendiente")

        # Checked
        sl = self.cs.parse_line("- [x] Tarea completada")
        self.assertEqual(sl.type, LineType.TASK_CHECKED)
        self.assertEqual(sl.text.decode('utf-8'), "Tarea completada")

        # Checked uppercase
        sl = self.cs.parse_line("- [X] Tarea completada 2")
        self.assertEqual(sl.type, LineType.TASK_CHECKED)
        self.assertEqual(sl.text.decode('utf-8'), "Tarea completada 2")

if __name__ == "__main__":
    unittest.main()
