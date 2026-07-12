# Gradient Knob — VST3 (Rust / nih-plug / egui)

A VST3 plugin whose editor is the "Circular Gradient Knob" UI, ported
directly from an `eframe`/`egui` standalone app into a real plugin using
nih-plug and its `nih_plug_egui` adapter. The drawing code in `src/ui.rs`
is line-for-line the same arc math, gradient, tick marks, glow, and knob
handle as the original `eframe::App`.

## Why this looks the way it does (Windows 7 constraint)

- eframe can't itself be a VST3 — it owns its own window/event loop.
  `nih_plug_egui` renders through egui-baseview, which uses OpenGL (glow),
  not wgpu/DX12 — that's what makes Windows 7 realistic at all.
- rustc dropped Windows 7 support starting at 1.78 (May 2024). The last
  stable release still targeting Windows 7/8/8.1 with the normal target
  is 1.77.2 — that's what CI pins to.
- nih-plug's own MSRV was bumped to 1.80 on 2024-08-18, so Cargo.toml pins
  a git commit resolved automatically in CI to the last commit before that
  date, rather than a tagged release.

This is a deliberately frozen, mid-2024 slice of the Rust/nih-plug
ecosystem. It won't get security updates or new features without breaking
Windows 7 support again.

## License note

`nih_export_vst3!()` links GPLv3-licensed VST3 bindings. Any VST3 built
from this project is subject to GPLv3 terms unless those bindings are
replaced with independently written ones.

## Testing on Windows 7

- Install the Visual C++ Redistributable matching the MSVC toolchain used
  to build.
- Confirm your DAW itself still supports Windows 7 — many modern DAW
  versions have dropped it independently of this plugin.
- OpenGL 3.2 core profile (what egui-baseview needs) is broadly available
  on Win7-era drivers, but test on real hardware where possible.