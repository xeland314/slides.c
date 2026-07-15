import unittest
import os
from c_slides_bindings import CSlides, LineType


class TestParser(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def test_parse_title(self):
        sl = self.cs.parse_line("# Mi Titulo")
        self.assertEqual(sl.type, LineType.TITLE)
        self.assertEqual(sl.text.decode("utf-8"), "Mi Titulo")

    def test_parse_subtitle(self):
        sl = self.cs.parse_line("## Mi Subtitulo")
        self.assertEqual(sl.type, LineType.SUBTITLE)
        self.assertEqual(sl.text.decode("utf-8"), "Mi Subtitulo")

    def test_parse_h3(self):
        sl = self.cs.parse_line("### Tercer nivel")
        self.assertEqual(sl.type, LineType.H3)
        self.assertEqual(sl.text.decode("utf-8"), "Tercer nivel")

    def test_parse_h4(self):
        sl = self.cs.parse_line("#### Cuarto nivel")
        self.assertEqual(sl.type, LineType.H4)
        self.assertEqual(sl.text.decode("utf-8"), "Cuarto nivel")

    def test_parse_h5(self):
        sl = self.cs.parse_line("##### Quinto nivel")
        self.assertEqual(sl.type, LineType.H5)
        self.assertEqual(sl.text.decode("utf-8"), "Quinto nivel")

    def test_parse_h6(self):
        sl = self.cs.parse_line("###### Sexto nivel")
        self.assertEqual(sl.type, LineType.H6)
        self.assertEqual(sl.text.decode("utf-8"), "Sexto nivel")

    def test_parse_h6_no_false_match(self):
        sl = self.cs.parse_line("####### Demasiado hashes")
        self.assertEqual(sl.type, LineType.BODY)
        sl = self.cs.parse_line("# #####:mixed")
        self.assertEqual(sl.type, LineType.TITLE)

    def test_parse_blockquote(self):
        sl = self.cs.parse_line("> Nota importante")
        self.assertEqual(sl.type, LineType.BLOCKQUOTE)
        self.assertEqual(sl.text.decode("utf-8"), "Nota importante")

    def test_parse_bullets(self):
        # Nivel 1
        sl = self.cs.parse_line("- Item 1")
        self.assertEqual(sl.type, LineType.BULLET1)
        self.assertEqual(sl.text.decode("utf-8"), "Item 1")

        # Nivel 2
        sl = self.cs.parse_line("  - Item 2")
        self.assertEqual(sl.type, LineType.BULLET2)
        self.assertEqual(sl.text.decode("utf-8"), "Item 2")

    def test_parse_image(self):
        # Formato corto
        sl = self.cs.parse_line("!logo.png")
        self.assertEqual(sl.type, LineType.IMAGE)
        self.assertEqual(sl.text.decode("utf-8"), "logo.png")

        # Formato Markdown completo
        sl = self.cs.parse_line("![alt text](path/to/img.png)")
        self.assertEqual(sl.type, LineType.IMAGE)
        self.assertEqual(sl.text.decode("utf-8"), "path/to/img.png")

    def test_parse_table(self):
        # Fila normal
        sl = self.cs.parse_line("| A | B |")
        self.assertEqual(sl.type, LineType.TABLE_ROW)
        self.assertEqual(sl.ncols, 2)
        self.assertEqual(sl.cols[0].value.decode("utf-8"), "A")
        self.assertEqual(sl.cols[1].value.decode("utf-8"), "B")

        # Separador
        sl = self.cs.parse_line("|---|---|")
        self.assertEqual(sl.type, LineType.TABLE_SEP)

    def test_parse_task_list(self):
        # Unchecked
        sl = self.cs.parse_line("- [ ] Tarea pendiente")
        self.assertEqual(sl.type, LineType.TASK_UNCHECKED)
        self.assertEqual(sl.text.decode("utf-8"), "Tarea pendiente")

        # Checked
        sl = self.cs.parse_line("- [x] Tarea completada")
        self.assertEqual(sl.type, LineType.TASK_CHECKED)
        self.assertEqual(sl.text.decode("utf-8"), "Tarea completada")

        # Checked uppercase
        sl = self.cs.parse_line("- [X] Tarea completada 2")
        self.assertEqual(sl.type, LineType.TASK_CHECKED)
        self.assertEqual(sl.text.decode("utf-8"), "Tarea completada 2")

    def test_parse_empty_line(self):
        sl = self.cs.parse_line("")
        self.assertEqual(sl.type, LineType.EMPTY)

    def test_parse_whitespace_only(self):
        sl = self.cs.parse_line("   ")
        self.assertEqual(sl.type, LineType.EMPTY)

    def test_parse_body_text(self):
        sl = self.cs.parse_line("Esto es un parrafo normal.")
        self.assertEqual(sl.type, LineType.BODY)
        self.assertEqual(sl.text.decode("utf-8"), "Esto es un parrafo normal.")

    def test_parse_body_long_text(self):
        long_text = "Palabra " * 50
        sl = self.cs.parse_line(long_text.strip())
        self.assertEqual(sl.type, LineType.BODY)
        self.assertEqual(sl.text.decode("utf-8"), long_text.strip())

    def test_parse_num_list_dot(self):
        sl = self.cs.parse_line("1. Primer elemento")
        self.assertEqual(sl.type, LineType.NUM_LIST)
        self.assertEqual(sl.marker.decode("utf-8"), "1.")
        self.assertEqual(sl.text.decode("utf-8"), "Primer elemento")

    def test_parse_num_list_letter(self):
        sl = self.cs.parse_line("a) Elemento con letra")
        self.assertEqual(sl.type, LineType.NUM_LIST)
        self.assertEqual(sl.marker.decode("utf-8"), "a)")
        self.assertEqual(sl.text.decode("utf-8"), "Elemento con letra")

    def test_parse_num_list_roman(self):
        sl = self.cs.parse_line("i. Numero romano")
        self.assertEqual(sl.type, LineType.NUM_LIST)
        self.assertEqual(sl.marker.decode("utf-8"), "i.")
        self.assertEqual(sl.text.decode("utf-8"), "Numero romano")

    def test_parse_num_list_uppercase(self):
        sl = self.cs.parse_line("A) Mayuscula")
        self.assertEqual(sl.type, LineType.NUM_LIST)
        self.assertEqual(sl.marker.decode("utf-8"), "A)")
        self.assertEqual(sl.text.decode("utf-8"), "Mayuscula")

    def test_parse_num_list_multi_digit(self):
        sl = self.cs.parse_line("12. Numero grande")
        self.assertEqual(sl.type, LineType.NUM_LIST)
        self.assertEqual(sl.marker.decode("utf-8"), "12.")
        self.assertEqual(sl.text.decode("utf-8"), "Numero grande")

    def test_parse_code_fence_start(self):
        # parse_line is a single-line parser; code fences are handled by slider_load.
        # ````c` doesn't match any specific rule, so it's treated as BODY by parse_line.
        sl = self.cs.parse_line("```c")
        self.assertEqual(sl.type, LineType.BODY)

    def test_parse_code_fence_end(self):
        # Same as above — bare ``` is treated as BODY by the line parser.
        sl = self.cs.parse_line("```")
        self.assertEqual(sl.type, LineType.BODY)

    def test_parse_table_multi_column(self):
        sl = self.cs.parse_line("| A | B | C | D |")
        self.assertEqual(sl.type, LineType.TABLE_ROW)
        self.assertEqual(sl.ncols, 4)
        self.assertEqual(sl.cols[0].value.decode("utf-8"), "A")
        self.assertEqual(sl.cols[1].value.decode("utf-8"), "B")
        self.assertEqual(sl.cols[2].value.decode("utf-8"), "C")
        self.assertEqual(sl.cols[3].value.decode("utf-8"), "D")

    def test_parse_table_with_alignment(self):
        # | :--- | ---: | :---: | is a separator (contains only |, -, :, space)
        sl = self.cs.parse_line("| :--- | ---: | :---: |")
        self.assertEqual(sl.type, LineType.TABLE_SEP)

    def test_parse_table_separator_variants(self):
        for sep in ["|---|---|", "| --- | --- |", "|:---|---:|", "| :--- | :---: |"]:
            with self.subTest(sep=sep):
                sl = self.cs.parse_line(sep)
                self.assertEqual(sl.type, LineType.TABLE_SEP)

    def test_parse_bullet_with_special_chars(self):
        sl = self.cs.parse_line("- Item con **negrita**")
        self.assertEqual(sl.type, LineType.BULLET1)
        self.assertEqual(sl.text.decode("utf-8"), "Item con **negrita**")

    def test_parse_image_markdown_format(self):
        sl = self.cs.parse_line("![descripcion larga](ruta/archivo.png)")
        self.assertEqual(sl.type, LineType.IMAGE)
        self.assertEqual(sl.text.decode("utf-8"), "ruta/archivo.png")

    def test_parse_blockquote_long(self):
        sl = self.cs.parse_line("> Esta es una cita muy larga que ocupara mucho espacio en la pantalla")
        self.assertEqual(sl.type, LineType.BLOCKQUOTE)
        self.assertTrue(len(sl.text.decode("utf-8")) > 20)

    def test_parse_title_with_special_chars(self):
        sl = self.cs.parse_line("# Titulo con符号 & <caracteres>")
        self.assertEqual(sl.type, LineType.TITLE)
        self.assertEqual(sl.text.decode("utf-8"), "Titulo con符号 & <caracteres>")

    def test_parse_subtitle_deep(self):
        sl = self.cs.parse_line("## Subtitulo profundo")
        self.assertEqual(sl.type, LineType.SUBTITLE)
        self.assertEqual(sl.text.decode("utf-8"), "Subtitulo profundo")


if __name__ == "__main__":
    unittest.main()
