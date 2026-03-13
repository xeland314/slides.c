const std = @import("std");
const c = @cImport({
    @cInclude("slider.h");
    @cInclude("src/ui/backend.h");
});

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    if (args.len < 2) {
        printHelp(args[0]);
        return;
    }

    var md_path: ?[]const u8 = null;
    var palette: ?[:0]const u8 = null;
    var font_family: ?[:0]const u8 = null;
    var font_scale: f64 = 1.0;
    var export_png: bool = false;
    var target_slide: i32 = -1;
    var export_w: i32 = 1080;
    var export_h: i32 = 1080;

    var i: usize = 1;
    while (i < args.len) : (i += 1) {
        const arg = args[i];
        if (std.mem.eql(u8, arg, "-p") or std.mem.eql(u8, arg, "--palette")) {
            i += 1;
            if (i < args.len) palette = try allocator.dupeZ(u8, args[i]);
        } else if (std.mem.eql(u8, arg, "-f") or std.mem.eql(u8, arg, "--font-family")) {
            i += 1;
            if (i < args.len) font_family = try allocator.dupeZ(u8, args[i]);
        } else if (std.mem.eql(u8, arg, "-s") or std.mem.eql(u8, arg, "--font-scale")) {
            i += 1;
            if (i < args.len) font_scale = try std.fmt.parseFloat(f64, args[i]);
        } else if (std.mem.eql(u8, arg, "-e") or std.mem.eql(u8, arg, "--export")) {
            export_png = true;
        } else if (std.mem.eql(u8, arg, "-er") or std.mem.eql(u8, arg, "--export-res")) {
            i += 1;
            if (i < args.len) {
                var it = std.mem.split(u8, args[i], "x");
                if (it.next()) |w_str| export_w = std.fmt.parseInt(i32, w_str, 10) catch 1080;
                if (it.next()) |h_str| export_h = std.fmt.parseInt(i32, h_str, 10) catch 1080;
            }
        } else if (std.mem.eql(u8, arg, "-sl") or std.mem.eql(u8, arg, "--slide")) {
            i += 1;
            if (i < args.len) target_slide = try std.fmt.parseInt(i32, args[i], 10);
        } else if (std.mem.eql(u8, arg, "-h") or std.mem.eql(u8, arg, "--help")) {
            printHelp(args[0]);
            return;
        } else {
            md_path = arg;
        }
    }

    if (md_path == null) {
        printHelp(args[0]);
        return;
    }

    const path_z = try allocator.dupeZ(u8, md_path.?);
    const slider = c.slider_load(path_z) orelse {
        std.debug.print("Error al cargar slides desde {s}\n", .{md_path.?});
        return;
    };
    defer c.slider_free(slider);

    if (palette) |p| c.slider_set_theme(slider, p);
    if (font_family) |f| c.slider_set_font_family(slider, f);
    if (font_scale != 1.0) c.slider_set_font_scale(slider, font_scale);

    const n_slides = c.slider_get_count(slider);
    const theme_name = std.mem.span(c.slider_get_theme_name(slider));
    const font_name = std.mem.span(c.slider_get_font_family(slider));
    const actual_scale = c.slider_get_font_scale(slider);

    std.debug.print("[slides-zig] {} slide(s) cargados\n", .{n_slides});
    std.debug.print("  Tema: {s}\n", .{theme_name});
    std.debug.print("  Font: {s}\n", .{font_name});
    std.debug.print("  Scale: {d:.1}\n", .{actual_scale});

    if (export_png) {
        const start = if (target_slide >= 0) target_slide else 0;
        const end = if (target_slide >= 0) target_slide + 1 else n_slides;

        if (start < 0 or start >= n_slides) {
            std.debug.print("Error: Slide {} fuera de rango (0-{})\n", .{target_slide, n_slides - 1});
            return;
        }

        var idx = start;
        while (idx < end) : (idx += 1) {
            var buf: [128]u8 = undefined;
            const filename = try std.fmt.bufPrintZ(&buf, "slide_zig_{}.png", .{idx + 1});
            const ret = c.slider_export_png(slider, idx, filename, export_w, export_h);
            if (ret == 0) {
                std.debug.print("Exportado: {s} ({}x{})\n", .{ filename, export_w, export_h });
            } else {
                std.debug.print("Fallo al exportar: {s}\n", .{filename});
            }
        }
        return;
    }

    std.debug.print("Iniciando backend Linux (X11) desde Zig...\n", .{});
    const ret = c.backend_run(slider);
    if (ret != 0) std.process.exit(@intCast(ret));
}

fn printHelp(prog: []const u8) void {
    std.debug.print("Uso: {s} [opciones] presentacion.md\n\n", .{prog});
    std.debug.print("Opciones:\n", .{});
    std.debug.print("  -p, --palette <name>    Elegir paleta (dark, rose, monokai, nord, light)\n", .{});
    std.debug.print("  -f, --font-family <str> Definir tipografía (ej. 'Arial', 'JetBrains Mono')\n", .{});
    std.debug.print("  -s, --font-scale <num>  Escalar tamaño de fuentes (ej. 1.2)\n", .{});
    std.debug.print("  -e, --export            Exportar slides a PNG\n", .{});
    std.debug.print("  -er, --export-res <WxH> Resolución de exportación (ej. 1920x1080)\n", .{});
    std.debug.print("  -sl, --slide <num>      Seleccionar slide específico para exportar (0-index)\n", .{});
    std.debug.print("  -h, --help              Mostrar esta ayuda\n", .{});
}
