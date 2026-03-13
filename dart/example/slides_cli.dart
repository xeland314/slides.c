import 'dart:ffi' as ffi;
import 'dart:io';
import 'package:ffi/ffi.dart';
import 'package:dart/c_slides_bindings.dart';

void printHelp() {
  print('Uso: slides_dart [opciones] presentacion.md\n');
  print('Opciones:');
  print('  -p, --palette <name>    Elegir paleta (dark, rose, monokai, nord, light)');
  print('  -f, --font-family <str> Definir tipografía (ej. \'Arial\', \'JetBrains Mono\')');
  print('  -s, --font-scale <num>  Escalar tamaño de fuentes (ej. 1.2)');
  print('  -e, --export            Exportar slides a PNG');
  print('  -er, --export-res <WxH> Resolución de exportación (ej. 1920x1080)');
  print('  -sl, --slide <num>      Seleccionar slide específico para exportar (0-index)');
  print('  -h, --help              Mostrar esta ayuda');
}

void main(List<String> args) {
  if (args.isEmpty) {
    printHelp();
    return;
  }

  String? mdPath;
  String? paletteName;
  String? fontFamily;
  double fontScale = 1.0;
  bool exportPng = false;
  int targetSlide = -1;
  int exportW = 1080;
  int exportH = 1080;

  for (int i = 0; i < args.length; i++) {
    final arg = args[i];
    if (arg == '--palette' || arg == '-p') {
      if (i + 1 < args.length) paletteName = args[++i];
    } else if (arg == '--font-family' || arg == '-f') {
      if (i + 1 < args.length) fontFamily = args[++i];
    } else if (arg == '--font-scale' || arg == '-s') {
      if (i + 1 < args.length) fontScale = double.tryParse(args[++i]) ?? 1.0;
    } else if (arg == '--export' || arg == '-e') {
      exportPng = true;
    } else if (arg == '--export-res' || arg == '-er') {
      if (i + 1 < args.length) {
        final resStr = args[++i];
        final resParts = resStr.split('x');
        if (resParts.length == 2) {
          exportW = int.tryParse(resParts[0]) ?? 1080;
          exportH = int.tryParse(resParts[1]) ?? 1080;
        }
      }
    } else if (arg == '--slide' || arg == '-sl') {
      if (i + 1 < args.length) targetSlide = int.tryParse(args[++i]) ?? -1;
    } else if (arg == '--help' || arg == '-h') {
      printHelp();
      return;
    } else {
      mdPath = arg;
    }
  }

  if (mdPath == null) {
    printHelp();
    return;
  }

  // Cargar librería. Buscamos en el directorio raíz.
  final libPath = File('../libslider.so').absolute.path;
  if (!File(libPath).existsSync()) {
    print('Error: No se encuentra libslider.so en $libPath');
    print('Ejecuta "make libslider.so" en el directorio raíz.');
    return;
  }

  final cslides = CSlides(libPath);

  final mdPathPtr = mdPath.toNativeUtf8();
  final slider = cslides.sliderLoad(mdPathPtr);
  malloc.free(mdPathPtr);

  if (slider == ffi.nullptr) {
    print('Error al cargar slides desde $mdPath');
    return;
  }

  if (paletteName != null) {
    final pPtr = paletteName.toNativeUtf8();
    cslides.sliderSetTheme(slider, pPtr);
    malloc.free(pPtr);
  }

  if (fontFamily != null) {
    final fPtr = fontFamily.toNativeUtf8();
    cslides.sliderSetFontFamily(slider, fPtr);
    malloc.free(fPtr);
  }

  if (fontScale != 1.0) {
    cslides.sliderSetFontScale(slider, fontScale);
  }

  final nSlides = cslides.sliderGetCount(slider);
  final themeName = cslides.sliderGetThemeName(slider).toDartString();
  final fontName = cslides.sliderGetFontFamily(slider).toDartString();
  final actualScale = cslides.sliderGetFontScale(slider);

  print('[slides-dart] $nSlides slide(s) cargados');
  print('  Tema: $themeName');
  print('  Font: $fontName');
  print('  Scale: ${actualScale.toStringAsFixed(1)}');

  if (exportPng) {
    final start = (targetSlide >= 0) ? targetSlide : 0;
    final end = (targetSlide >= 0) ? targetSlide + 1 : nSlides;

    if (start < 0 || start >= nSlides) {
      print('Error: Slide $targetSlide fuera de rango (0-${nSlides - 1})');
      cslides.sliderFree(slider);
      return;
    }

    for (int i = start; i < end; i++) {
      final filename = 'slide_dart_${i + 1}.png';
      final fPtr = filename.toNativeUtf8();
      final ret = cslides.sliderExportPng(slider, i, fPtr, exportW, exportH);
      malloc.free(fPtr);
      if (ret == 0) {
        print('Exportado: $filename (${exportW}x${exportH})');
      } else {
        print('Fallo al exportar: $filename');
      }
    }
    cslides.sliderFree(slider);
    return;
  }

  print('Iniciando backend Linux (X11) desde Dart...');
  final ret = cslides.backendRun(slider);
  
  cslides.sliderFree(slider);
  exit(ret);
}
