import sys
import os
import argparse
from c_slides_bindings import CSlides

def main():
    parser = argparse.ArgumentParser(description="Port de c-slides en Python")
    parser.add_argument("md_path", help="Ruta al archivo markdown")
    parser.add_argument("-p", "--palette", help="Elegir paleta (dark, rose, monokai, nord, light)")
    parser.add_argument("-f", "--font-family", help="Definir tipografía")
    parser.add_argument("-s", "--font-scale", type=float, default=1.0, help="Escalar tamaño de fuentes")
    parser.add_argument("-e", "--export", action="store_true", help="Exportar slides a PNG")
    parser.add_argument("-er", "--export-res", default="1080x1080", help="Resolución de exportación (ej. 1920x1080)")
    parser.add_argument("-sl", "--slide", type=int, default=-1, help="Seleccionar slide específico para exportar")

    args = parser.parse_args()

    # Buscar la librería libslider.so en el directorio raíz
    lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "libslider.so"))
    
    try:
        cslides = CSlides(lib_path)
    except FileNotFoundError as e:
        print(f"Error: {e}")
        print("Asegúrate de haber ejecutado 'make libslider.so' en el directorio raíz.")
        sys.exit(1)

    slider = cslides.load(args.md_path)
    if not slider:
        print(f"Error al cargar slides desde {args.md_path}")
        sys.exit(1)

    if args.palette:
        cslides.set_theme(slider, args.palette)
    if args.font_family:
        cslides.set_font_family(slider, args.font_family)
    if args.font_scale != 1.0:
        cslides.set_font_scale(slider, args.font_scale)

    n_slides = cslides.get_count(slider)
    theme_name = cslides.get_theme_name(slider)
    font_name = cslides.get_font_family(slider)
    scale = cslides.get_font_scale(slider)

    print(f"[slides-python] {n_slides} slide(s) cargados")
    print(f"  Tema: {theme_name}")
    print(f"  Font: {font_name}")
    print(f"  Scale: {scale:.1f}")

    if args.export:
        try:
            export_w, export_h = map(int, args.export_res.lower().split('x'))
        except ValueError:
            print("Error: El formato de resolución debe ser WxH (ej. 1920x1080)")
            cslides.free(slider)
            sys.exit(1)

        start = args.slide if args.slide >= 0 else 0
        end = args.slide + 1 if args.slide >= 0 else n_slides

        if start < 0 or start >= n_slides:
            print(f"Error: Slide {args.slide} fuera de rango (0-{n_slides-1})")
            cslides.free(slider)
            sys.exit(1)

        for i in range(start, end):
            filename = f"slide_python_{i+1}.png"
            ret = cslides.export_png(slider, i, filename, export_w, export_h)
            if ret == 0:
                print(f"Exportado: {filename} ({export_w}x{export_h})")
            else:
                print(f"Fallo al exportar: {filename}")
        
        cslides.free(slider)
        sys.exit(0)

    print("Iniciando backend Linux (X11) desde Python...")
    ret = cslides.run_backend(slider)
    
    cslides.free(slider)
    sys.exit(ret)

if __name__ == "__main__":
    main()
