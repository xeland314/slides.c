local ffi = require("ffi")

-- 1. Detección de OS y carga de librería
local is_windows = package.config:sub(1,1) == "\\"
local lib_name = is_windows and "../slider.dll" or "../libslider.so"

-- Se intenta cargar la librería
local status, lib = pcall(ffi.load, lib_name)
if not status then
    print("Error: No se pudo cargar la librería en: " .. lib_name)
    print("Asegúrate de haber ejecutado 'make' en el directorio raíz.")
    os.exit(1)
end

-- Declaraciones de C
ffi.cdef[[
    typedef struct Slider Slider;
    Slider* slider_load(const char *path);
    void slider_free(Slider *s);
    int slider_get_count(Slider *s);
    int backend_run(Slider *s);
    int slider_export_png(Slider *s, int index, const char *path, int w, int h);
    void slider_set_theme(Slider *s, const char *theme_name);
    void slider_set_font_family(Slider *s, const char *font_family);
    void slider_set_font_scale(Slider *s, double font_scale);
    const char* slider_get_theme_name(Slider *s);
    const char* slider_get_font_family(Slider *s);
    double slider_get_font_scale(Slider *s);
]]

local function print_help()
    print("Uso: luajit main.lua [opciones] presentacion.md\n")
    print("Opciones:")
    print("  -p, --palette <name>    Elegir paleta (dark, rose, monokai, nord, light)")
    print("  -f, --font-family <str> Definir tipografía")
    print("  -s, --font-scale <num>  Escalar tamaño de fuentes")
    print("  -e, --export            Exportar slides a PNG")
    print("  -er, --export-res <WxH> Resolución de exportación")
    print("  -sl, --slide <num>      Seleccionar slide específico")
    print("  -h, --help              Mostrar esta ayuda")
end

-- Lógica de argumentos
local md_path = nil
local palette = nil
local font_family = nil
local font_scale = 1.0
local export_png = false
local target_slide = -1
local export_w = 1080
local export_h = 1080

local i = 1
while i <= #arg do
    if arg[i] == "-p" or arg[i] == "--palette" then
        i = i + 1; palette = arg[i]
    elseif arg[i] == "-f" or arg[i] == "--font-family" then
        i = i + 1; font_family = arg[i]
    elseif arg[i] == "-s" or arg[i] == "--font-scale" then
        i = i + 1; font_scale = tonumber(arg[i]) or 1.0
    elseif arg[i] == "-e" or arg[i] == "--export" then
        export_png = true
    elseif arg[i] == "-er" or arg[i] == "--export-res" then
        i = i + 1
        if arg[i] then
            local w, h = arg[i]:match("(%d+)x(%d+)")
            export_w = tonumber(w) or 1080
            export_h = tonumber(h) or 1080
        end
    elseif arg[i] == "-sl" or arg[i] == "--slide" then
        i = i + 1; target_slide = tonumber(arg[i]) or -1
    elseif arg[i] == "-h" or arg[i] == "--help" then
        print_help()
        os.exit(0)
    else
        md_path = arg[i]
    end
    i = i + 1
end

if not md_path then
    print_help()
    os.exit(1)
end

local slider = lib.slider_load(md_path)
if slider == nil then
    print("Error al cargar slides: " .. md_path)
    os.exit(1)
end

-- Aplicar settings
if palette then lib.slider_set_theme(slider, palette) end
if font_family then lib.slider_set_font_family(slider, font_family) end
if font_scale ~= 1.0 then lib.slider_set_font_scale(slider, font_scale) end

local n_slides = lib.slider_get_count(slider)
local theme_name = ffi.string(lib.slider_get_theme_name(slider))
local font_name = ffi.string(lib.slider_get_font_family(slider))
local actual_scale = lib.slider_get_font_scale(slider)

print(string.format("[slides-lua] %d slide(s) cargados", n_slides))
print("  Tema: " .. theme_name)
print("  Font: " .. font_name)
print(string.format("  Scale: %.1f", actual_scale))

if export_png then
    local start_idx = (target_slide >= 0) and target_slide or 0
    local end_idx = (target_slide >= 0) and (target_slide + 1) or n_slides

    for idx = start_idx, end_idx - 1 do
        local filename = string.format("slide_lua_%d.png", idx + 1)
        local ret = lib.slider_export_png(slider, idx, filename, export_w, export_h)
        if ret == 0 then
            print(string.format("Exportado: %s (%dx%d)", filename, export_w, export_h))
        else
            print("Fallo al exportar: " .. filename)
        end
    end
    lib.slider_free(slider)
    os.exit(0)
end

local backend_name = is_windows and "Win32" or "Linux (X11)"
print("Iniciando backend " .. backend_name .. " desde Lua...")
local ret = lib.backend_run(slider)

lib.slider_free(slider)
os.exit(ret)
