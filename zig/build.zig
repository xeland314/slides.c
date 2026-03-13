const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "slides_zig",
        .root_module = b.createModule(.{
            .root_source_file = b.path("main.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });

    // Directorios de inclusión (ajusta según tu estructura)
    exe.addIncludePath(b.path(".."));

    // Enlace con la librería core
    exe.addLibraryPath(b.path(".."));
    exe.linkSystemLibrary("slider");

    // Enlace con dependencias de Cairo/Pango
    exe.linkSystemLibrary("pangocairo-1.0");
    exe.linkSystemLibrary("pango-1.0");
    exe.linkSystemLibrary("gobject-2.0");
    exe.linkSystemLibrary("glib-2.0");
    exe.linkSystemLibrary("cairo");

    // Dependencias específicas del SO
    if (target.result.os.tag == .windows) {
        exe.linkSystemLibrary("gdi32");
        exe.linkSystemLibrary("user32");
    } else {
        exe.linkSystemLibrary("X11");
    }

    exe.linkLibC();
    b.installArtifact(exe);
}
