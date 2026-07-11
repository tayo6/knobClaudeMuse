# Gradient Knob — VST3 (Rust / nih-plug / egui)

A VST3 plugin whose editor is the "Circular Gradient Knob" UI, ported
directly from an `eframe`/`egui` standalone app into a real plugin using
[`nih-plug`](https://github.com/robbert-vdh/nih-plug) and its `nih_plug_egui`
adapter. The drawing code in `src/ui.rs` is line-for-line the same arc math,
gradient, tick marks, glow, and knob handle as the original `eframe::App`.

## Why this looks the way it does (Windows 7 constraint)

You asked for this to run on **Windows 7**. That constraint drove several
non-obvious decisions:

- **eframe itself can't be a VST3.** eframe owns its own window and event
  loop; it can't be embedded as a plugin editor. `nih_plug_egui` renders
  through `egui-baseview`, which uses **OpenGL (glow)**, not `wgpu`/DX12 —
  that's what makes Windows 7 realistic at all, since wgpu's modern backends
  assume a much newer OS/driver stack.
- **rustc itself dropped Windows 7.** Starting with rustc **1.78** (May 2024),
  the standard `*-pc-windows-msvc` targets require Windows 10+. The last
  stable release that still targets Windows 7/8/8.1 with the normal target
  is **1.77.2** — that's what this project's CI pins to.
  ([Rust blog announcement](https://blog.rust-lang.org/2024/02/26/Windows-7/))
- **nih-plug outgrew that toolchain.** nih-plug's own MSRV was bumped to 1.80
  on 2024-08-18. So `Cargo.toml` doesn't pin a released nih-plug version —
  it pins a **git commit resolved automatically in CI** to the last commit
  on `master` before that MSRV bump, so the pinned 1.77.2 toolchain can
  actually compile it. See `.github/workflows/build.yml`, step "Resolve
  Win7-compatible nih-plug commit".

**In short: this is an old, frozen slice of the Rust/nih-plug ecosystem,
deliberately, because that's what Windows 7 requires.** It will not get
security updates, new nih-plug features, or newer egui versions without
breaking the Windows 7 target again.

## License note (please read before distributing)

`nih_export_vst3!()` — required to produce a VST3 — links **GPLv3-licensed**
VST3 bindings. Unless those bindings are replaced with independently written
ones, any VST3 built from this project is subject to GPLv3 terms. This is
upstream nih-plug's stated policy, not something this project adds.

## Building locally (optional — GitHub Actions does this for you)

You generally don't need to do this yourself; push to `main` or run the
workflow manually ("Run workflow" in the Actions tab) and download the
`GradientKnob-VST3-Windows7` artifact when it finishes.

If you do want to build locally on Windows:

```powershell
rustup toolchain install 1.77.2 --target x86_64-pc-windows-msvc
# Resolve and patch the nih-plug commit the same way CI does — see
# .github/workflows/build.yml for the exact git log command — then:
cargo +1.77.2 build --release --target x86_64-pc-windows-msvc
```

The resulting DLL needs to be placed inside a `.vst3` bundle folder
structure (`GradientKnob.vst3/Contents/x86_64-win/GradientKnob.vst3`) — the
CI workflow does this assembly step for you.

## Testing on actual Windows 7

A few things worth knowing before you test:

- Install the **Visual C++ Redistributable** matching the MSVC toolchain
  used to build (VS 2015-2022 redistributable covers this).
  Without it, the DLL will fail to load with a generic error in the host.
- Confirm your **DAW itself** still supports Windows 7 — many modern DAW
  versions (recent Ableton, recent Cubase, etc.) have themselves dropped
  Windows 7, independent of this plugin. A lightweight, actively-maintained
  Win7-era host (e.g. an older REAPER release) is the most reliable way to
  isolate "does the plugin work" from "does the DAW even run."
- OpenGL 3.2 core profile (what `egui-baseview` needs) has been broadly
  available since Windows 7-era GPU drivers, but very old or minimal
  virtual-machine GPU drivers can lack it — test on real hardware if
  possible, or a VM with full GPU passthrough rather than a software
  renderer.

## Project layout

```
Cargo.toml                     — pinned dependencies (rev filled in by CI)
src/lib.rs                     — Plugin/Params/Vst3Plugin impls, editor wiring
src/ui.rs                      — ported drawing code (arc, gradient, ticks, knob)
.github/workflows/build.yml    — resolves commit, builds, bundles, uploads artifact
```
