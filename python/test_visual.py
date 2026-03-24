import unittest
import os
from c_slides_bindings import CSlides

class TestVisual(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "libslider.so"))
        cls.cs = CSlides(lib_path)

    def test_export_png(self):
        # Cargar un ejemplo básico
        md_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "examples", "basic.md"))
        out_png = os.path.abspath(os.path.join(os.path.dirname(__file__), "test_output.png"))
        
        if os.path.exists(out_png):
            os.remove(out_png)

        s = self.cs.load(md_path)
        self.assertTrue(bool(s))
        
        # Exportar la primera slide
        res = self.cs.export_png(s, 0, out_png, 800, 600)
        self.assertEqual(res, 0, "slider_export_png falló")
        
        # Verificar que el archivo existe y tiene tamaño
        self.assertTrue(os.path.exists(out_png))
        self.assertGreater(os.path.getsize(out_png), 1000) # Un PNG mínimo debería pesar algo
        
        self.cs.free(s)
        
        # Limpiar
        if os.path.exists(out_png):
            os.remove(out_png)

    def test_export_all_themes(self):
        # Test de stress: cambiar temas y exportar
        md_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "examples", "basic.md"))
        themes = ["dark", "light", "dracula", "gruvbox", "monokai", "nord", "rose", "blue", "tokyo-night", "catppuccin", "ambercat"]
        
        s = self.cs.load(md_path)
        
        for theme in themes:
            with self.subTest(theme=theme):
                self.cs.set_theme(s, theme)
                out_png = os.path.abspath(os.path.join(os.path.dirname(__file__), f"test_{theme}.png"))
                res = self.cs.export_png(s, 0, out_png, 400, 300)
                self.assertEqual(res, 0, f"Falló exportación con tema {theme}")
                self.assertTrue(os.path.exists(out_png))
                os.remove(out_png)
        
        self.cs.free(s)

if __name__ == "__main__":
    unittest.main()
