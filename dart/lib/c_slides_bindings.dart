import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';
import 'dart:io';

// Definición del tipo opaco Slider
base class Slider extends ffi.Opaque {}

// Tipos de funciones para FFI
typedef SliderLoadC = ffi.Pointer<Slider> Function(ffi.Pointer<Utf8> path);
typedef SliderLoadDart = ffi.Pointer<Slider> Function(ffi.Pointer<Utf8> path);

typedef SliderFreeC = ffi.Void Function(ffi.Pointer<Slider> s);
typedef SliderFreeDart = void Function(ffi.Pointer<Slider> s);

typedef SliderGetCountC = ffi.Int32 Function(ffi.Pointer<Slider> s);
typedef SliderGetCountDart = int Function(ffi.Pointer<Slider> s);

typedef BackendRunC = ffi.Int32 Function(ffi.Pointer<Slider> s);
typedef BackendRunDart = int Function(ffi.Pointer<Slider> s);

typedef SliderExportPngC = ffi.Int32 Function(
    ffi.Pointer<Slider> s, ffi.Int32 index, ffi.Pointer<Utf8> path, ffi.Int32 w, ffi.Int32 h);
typedef SliderExportPngDart = int Function(
    ffi.Pointer<Slider> s, int index, ffi.Pointer<Utf8> path, int w, int h);

typedef SliderSetThemeC = ffi.Void Function(ffi.Pointer<Slider> s, ffi.Pointer<Utf8> themeName);
typedef SliderSetThemeDart = void Function(ffi.Pointer<Slider> s, ffi.Pointer<Utf8> themeName);

typedef SliderSetFontFamilyC = ffi.Void Function(ffi.Pointer<Slider> s, ffi.Pointer<Utf8> fontFamily);
typedef SliderSetFontFamilyDart = void Function(ffi.Pointer<Slider> s, ffi.Pointer<Utf8> fontFamily);

typedef SliderSetFontScaleC = ffi.Void Function(ffi.Pointer<Slider> s, ffi.Double fontScale);
typedef SliderSetFontScaleDart = void Function(ffi.Pointer<Slider> s, double fontScale);

typedef SliderGetThemeNameC = ffi.Pointer<Utf8> Function(ffi.Pointer<Slider> s);
typedef SliderGetThemeNameDart = ffi.Pointer<Utf8> Function(ffi.Pointer<Slider> s);

typedef SliderGetFontFamilyC = ffi.Pointer<Utf8> Function(ffi.Pointer<Slider> s);
typedef SliderGetFontFamilyDart = ffi.Pointer<Utf8> Function(ffi.Pointer<Slider> s);

typedef SliderGetFontScaleC = ffi.Double Function(ffi.Pointer<Slider> s);
typedef SliderGetFontScaleDart = double Function(ffi.Pointer<Slider> s);

class CSlides {
  late final ffi.DynamicLibrary _lib;

  late final SliderLoadDart sliderLoad;
  late final SliderFreeDart sliderFree;
  late final SliderGetCountDart sliderGetCount;
  late final BackendRunDart backendRun;
  late final SliderExportPngDart sliderExportPng;
  late final SliderSetThemeDart sliderSetTheme;
  late final SliderSetFontFamilyDart sliderSetFontFamily;
  late final SliderSetFontScaleDart sliderSetFontScale;
  late final SliderGetThemeNameDart sliderGetThemeName;
  late final SliderGetFontFamilyDart sliderGetFontFamily;
  late final SliderGetFontScaleDart sliderGetFontScale;

  CSlides(String path) {
    _lib = ffi.DynamicLibrary.open(path);

    sliderLoad = _lib
        .lookup<ffi.NativeFunction<SliderLoadC>>('slider_load')
        .asFunction();

    sliderFree = _lib
        .lookup<ffi.NativeFunction<SliderFreeC>>('slider_free')
        .asFunction();

    sliderGetCount = _lib
        .lookup<ffi.NativeFunction<SliderGetCountC>>('slider_get_count')
        .asFunction();

    backendRun = _lib
        .lookup<ffi.NativeFunction<BackendRunC>>('backend_run')
        .asFunction();

    sliderExportPng = _lib
        .lookup<ffi.NativeFunction<SliderExportPngC>>('slider_export_png')
        .asFunction();

    sliderSetTheme = _lib
        .lookup<ffi.NativeFunction<SliderSetThemeC>>('slider_set_theme')
        .asFunction();

    sliderSetFontFamily = _lib
        .lookup<ffi.NativeFunction<SliderSetFontFamilyC>>('slider_set_font_family')
        .asFunction();

    sliderSetFontScale = _lib
        .lookup<ffi.NativeFunction<SliderSetFontScaleC>>('slider_set_font_scale')
        .asFunction();

    sliderGetThemeName = _lib
        .lookup<ffi.NativeFunction<SliderGetThemeNameC>>('slider_get_theme_name')
        .asFunction();

    sliderGetFontFamily = _lib
        .lookup<ffi.NativeFunction<SliderGetFontFamilyC>>('slider_get_font_family')
        .asFunction();

    sliderGetFontScale = _lib
        .lookup<ffi.NativeFunction<SliderGetFontScaleC>>('slider_get_font_scale')
        .asFunction();
  }
}
