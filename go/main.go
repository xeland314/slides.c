package main

/*
#cgo CFLAGS: -I..
#cgo linux LDFLAGS: -L.. -lslider -lX11 -lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lharfbuzz -lcairo -lm
#cgo windows LDFLAGS: -L.. -lslider -lgdi32 -luser32 -lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lharfbuzz -lcairo -lm
#include "slider.h"
#include "src/ui/backend.h"
#include <stdlib.h>
*/
import "C"
import (
	"flag"
	"fmt"
	"os"
	"runtime"
	"unsafe"
)

type Config struct {
	mdPath      string
	palette     string
	fontFamily  string
	fontScale   float64
	export      bool
	exportRes   string
	targetSlide int
}

func parseFlags() Config {
	c := Config{}
	flag.StringVar(&c.palette, "palette", "", "Elegir paleta")
	flag.StringVar(&c.palette, "p", "", "Elegir paleta (shorthand)")
	flag.StringVar(&c.fontFamily, "font-family", "", "Definir tipografía")
	flag.StringVar(&c.fontFamily, "f", "", "Definir tipografía (shorthand)")
	flag.Float64Var(&c.fontScale, "font-scale", 1.0, "Escalar tamaño de fuentes")
	flag.Float64Var(&c.fontScale, "s", 1.0, "Escalar tamaño de fuentes (shorthand)")
	flag.BoolVar(&c.export, "export", false, "Exportar slides a PNG")
	flag.BoolVar(&c.export, "e", false, "Exportar slides a PNG (shorthand)")
	flag.StringVar(&c.exportRes, "export-res", "1080x1080", "Resolución (ej. 1920x1080)")
	flag.StringVar(&c.exportRes, "er", "1080x1080", "Resolución (shorthand)")
	flag.IntVar(&c.targetSlide, "slide", -1, "Slide específico")
	flag.IntVar(&c.targetSlide, "sl", -1, "Slide específico (shorthand)")

	flag.Parse()
	if flag.NArg() < 1 {
		fmt.Printf("Uso: %s [opciones] presentacion.md\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(0)
	}
	c.mdPath = flag.Arg(0)
	return c
}

func applySettings(slider *C.Slider, c Config) {
	if c.palette != "" {
		cp := C.CString(c.palette)
		defer C.free(unsafe.Pointer(cp))
		C.slider_set_theme(slider, cp)
	}
	if c.fontFamily != "" {
		cf := C.CString(c.fontFamily)
		defer C.free(unsafe.Pointer(cf))
		C.slider_set_font_family(slider, cf)
	}
	if c.fontScale != 1.0 {
		C.slider_set_font_scale(slider, C.double(c.fontScale))
	}
}

func runExport(slider *C.Slider, c Config, nSlides int) {
	var w, h int
	fmt.Sscanf(c.exportRes, "%dx%d", &w, &h)

	start, end := 0, nSlides
	if c.targetSlide >= 0 {
		start, end = c.targetSlide, c.targetSlide+1
	}

	if start < 0 || start >= nSlides {
		fmt.Printf("Error: Slide %d fuera de rango\n", c.targetSlide)
		return
	}

	for i := start; i < end; i++ {
		name := fmt.Sprintf("slide_go_%d.png", i+1)
		cn := C.CString(name)
		ret := C.slider_export_png(slider, C.int(i), cn, C.int(w), C.int(h))
		C.free(unsafe.Pointer(cn))

		if ret == 0 {
			fmt.Printf("Exportado: %s (%dx%d)\n", name, w, h)
		} else {
			fmt.Printf("Fallo al exportar: %s\n", name)
		}
	}
}

func main() {
	config := parseFlags()

	cPath := C.CString(config.mdPath)
	defer C.free(unsafe.Pointer(cPath))

	slider := C.slider_load(cPath)
	if slider == nil {
		fmt.Printf("Error al cargar %s\n", config.mdPath)
		return
	}
	defer C.slider_free(slider)

	applySettings(slider, config)

	nSlides := int(C.slider_get_count(slider))
	fmt.Printf("[slides-go] %d slides cargados | Tema: %s | SO: %s\n",
		nSlides, C.GoString(C.slider_get_theme_name(slider)), runtime.GOOS)

	if config.export {
		runExport(slider, config, nSlides)
		return
	}

	fmt.Printf("Iniciando backend %s...\n", runtime.GOOS)
	C.backend_run(slider)
}
