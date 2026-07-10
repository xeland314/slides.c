import argparse
import platform
import os
import sys
from c_slides_bindings import CSlides

def setup_environment():
    """Configura las rutas de las DLLs y devuelve el path de la librería."""
    if platform.system() == "Windows":
        msys_path = r"C:\msys64\mingw64\bin"
        if os.path.exists(msys_path):
            os.add_dll_directory(msys_path)
        else:
            print(f"Advertencia: No se encontró MSYS2 en {msys_path}")

    ext = ".dll" if platform.system() == "Windows" else ".so"
    lib_name = f"slider{ext}"
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", lib_name))

def parse_arguments():
    """Define y parsea los argumentos de línea de comandos."""
    parser = argparse.ArgumentParser(description="Port de c-slides en Python")
    parser.add_argument("md_path", help="Ruta al archivo markdown")
    parser.add_argument("-p", "--palette", help="Elegir paleta (dark, rose, monokai, nord, light)")
    parser.add_argument("-f", "--font-family", help="Definir tipografía")
    parser.add_argument("-s", "--font-scale", type=float, default=1.0, help="Escalar tamaño de fuentes")
    parser.add_argument("-e", "--export", action="store_true", help="Exportar slides a PNG")
    parser.add_argument("-er", "--export-res", default="1080x1080", help="Resolución (ej. 1920x1080)")
    parser.add_argument("-sl", "--slide", type=int, default=-1, help="Slide específico")
    return parser.parse_args()

def handle_export(cslides, slider, args, n_slides):
    """Lógica para la exportación de imágenes PNG."""
    try:
        export_w, export_h = map(int, args.export_res.lower().split('x'))
    except ValueError:
        print("Error: Formato de resolución inválido.")
        return 1

    start = args.slide if args.slide >= 0 else 0
    end = args.slide + 1 if args.slide >= 0 else n_slides

    if start < 0 or start >= n_slides:
        print(f"Error: Slide {args.slide} fuera de rango.")
        return 1

    for i in range(start, end):
        filename = f"slide_python_{i+1}.png"
        if cslides.export_png(slider, i, filename, export_w, export_h) == 0:
            print(f"Exportado: {filename} ({export_w}x{export_h})")
        else:
            print(f"Fallo al exportar: {filename}")
    return 0

def main():
    args = parse_arguments()
    lib_path = setup_environment()

    try:
        cslides = CSlides(lib_path)
    except FileNotFoundError as e:
        print(f"Error: {e}\nAsegúrate de compilar la librería dinámica.")
        sys.exit(1)

    slider = cslides.load(args.md_path)
    if not slider:
        print(f"Error al cargar slides desde {args.md_path}")
        sys.exit(1)

    # Configuración de estilo
    if args.palette: cslides.set_theme(slider, args.palette)
    if args.font_family: cslides.set_font_family(slider, args.font_family)
    if args.font_scale != 1: cslides.set_font_scale(slider, args.font_scale)

    n_slides = cslides.get_count(slider)
    print(f"[slides-python] {n_slides} slide(s) cargados ({cslides.get_theme_name(slider)})")

    if args.export:
        status = handle_export(cslides, slider, args, n_slides)
        cslides.free(slider)
        sys.exit(status)

    # Modo presentación
    print(f"Iniciando backend {platform.system()}...")
    ret = cslides.run_backend(slider)
    
    cslides.free(slider)
    sys.exit(ret)

if __name__ == "__main__":
    main()
