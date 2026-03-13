package main

/*
#cgo CFLAGS: -I..
#cgo LDFLAGS: -L.. -lslider -lX11 -lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lharfbuzz -lcairo -lm
#include "slider.h"
#include "src/ui/backend.h"
#include <stdlib.h>
*/
import "C"
import (
	"flag"
	"fmt"
	"os"
	"unsafe"
)

func main() {
	paletteName := flag.String("palette", "", "Elegir paleta")
	flag.StringVar(paletteName, "p", "", "Elegir paleta (shorthand)")
	fontFamily := flag.String("font-family", "", "Definir tipografía")
	flag.StringVar(fontFamily, "f", "", "Definir tipografía (shorthand)")
	fontScale := flag.Float64("font-scale", 1.0, "Escalar tamaño de fuentes")
	flag.Float64Var(fontScale, "s", 1.0, "Escalar tamaño de fuentes (shorthand)")
	exportPng := flag.Bool("export", false, "Exportar slides a PNG")
	flag.BoolVar(exportPng, "e", false, "Exportar slides a PNG (shorthand)")
	exportRes := flag.String("export-res", "1080x1080", "Resolución de exportación (ej. 1920x1080)")
	flag.StringVar(exportRes, "er", "1080x1080", "Resolución de exportación (shorthand)")
	targetSlide := flag.Int("slide", -1, "Seleccionar slide específico")
	flag.IntVar(targetSlide, "sl", -1, "Seleccionar slide específico (shorthand)")

	flag.Parse()

	if flag.NArg() < 1 {
		fmt.Printf("Uso: %s [opciones] presentacion.md\n", os.Args[0])
		flag.PrintDefaults()
		return
	}

	mdPath := flag.Arg(0)
	cPath := C.CString(mdPath)
	defer C.free(unsafe.Pointer(cPath))

	slider := C.slider_load(cPath)
	if slider == nil {
		fmt.Printf("Error al cargar slides desde %s\n", mdPath)
		return
	}
	defer C.slider_free(slider)

	if *paletteName != "" {
		cP := C.CString(*paletteName)
		C.slider_set_theme(slider, cP)
		C.free(unsafe.Pointer(cP))
	}

	if *fontFamily != "" {
		cF := C.CString(*fontFamily)
		C.slider_set_font_family(slider, cF)
		C.free(unsafe.Pointer(cF))
	}

	if *fontScale != 1.0 {
		C.slider_set_font_scale(slider, C.double(*fontScale))
	}

	nSlides := int(C.slider_get_count(slider))
	themeName := C.GoString(C.slider_get_theme_name(slider))
	fontName := C.GoString(C.slider_get_font_family(slider))
	actualScale := float64(C.slider_get_font_scale(slider))

	fmt.Printf("[slides-go] %d slide(s) cargados\n", nSlides)
	fmt.Printf("  Tema: %s\n", themeName)
	fmt.Printf("  Font: %s\n", fontName)
	fmt.Printf("  Scale: %.1f\n", actualScale)

	if *exportPng {
		exportW, exportH := 1080, 1080
		fmt.Sscanf(*exportRes, "%dx%d", &exportW, &exportH)

		start := 0
		end := nSlides
		if *targetSlide >= 0 {
			start = *targetSlide
			end = *targetSlide + 1
		}

		if start < 0 || start >= nSlides {
			fmt.Printf("Error: Slide %d fuera de rango\n", *targetSlide)
			return
		}

		for i := start; i < end; i++ {
			filename := fmt.Sprintf("slide_go_%d.png", i+1)
			cFilename := C.CString(filename)
			ret := C.slider_export_png(slider, C.int(i), cFilename, C.int(exportW), C.int(exportH))
			C.free(unsafe.Pointer(cFilename))

			if ret == 0 {
				fmt.Printf("Exportado: %s (%dx%d)\n", filename, exportW, exportH)
			} else {
				fmt.Printf("Fallo al exportar: %s\n", filename)
			}
		}
		return
	}

	fmt.Println("Iniciando backend Linux (X11) desde Go...")
	ret := C.backend_run(slider)
	if ret != 0 {
		os.Exit(int(ret))
	}
}
