# Groove Machine Mobile native blob overview

This repository originally contained a single 97k‑line decompiled file (`libgroovemachinemobile.so.c`). The JNI entry points, engine lifecycle hooks and file format handling have been identified and documented below.

## JNI entry points and rendering/audio bridge

* OpenGL initialization and resize feed the engine with the current viewport and then start audio: `Java_com_imageline_GrooveMachineMobile_ILRenderer_nativeInitGL`, `Java_com_imageline_GrooveMachineMobile_ILRenderer_nativeResize`, and the render loop `Java_com_imageline_GrooveMachineMobile_ILRenderer_nativeRender` call into `Engine_SetScreenSize`, `Engine_Create`, `Engine_StartAudioHard`, and `Engine_RenderGraphics`.【F:libgroovemachinemobile.so.c†L57152-L57238】
* App lifecycle hooks map directly to engine calls: `nativeUIThreadIdle`, `nativeDone`, `nativePause`, and `nativeResume` wrap `Engine_UIThreadIdle`, `Engine_Destroy`/`Engine_StopAudioHard`, `Engine_Suspend`, and `Engine_StartAudioHard`.【F:libgroovemachinemobile.so.c†L57176-L57200】
* Touch input is funneled through `Engine_Touch` with a phase (begin/move/end), coordinates and a 1‑based pointer id: see `nativeTouchBegan/Move/End`.【F:libgroovemachinemobile.so.c†L57215-L57231】
* Asset enumeration and a small startup read are performed through the Android `AAssetManager` and forwarded to the engine (`Engine_FileFound`).【F:libgroovemachinemobile.so.c†L57203-L57248】

## File format handling

* UI file browser filters map extensions to different event buffers:
  * `flgsynth` and `ini` (sampled instruments) share one channel,
  * `flgsample` uses another,
  * `spectra` is treated separately (likely spectral data),
  * `flgroove` files go to the groove/song loader,
  * `wav` files are special‑cased for drum samples or user samples depending on their directory (install `Drum Samples` vs. user `MySamples`).【F:libgroovemachinemobile.so.c†L17790-L17839】
* Loading presets: when a `.flgsynth` or `.flgsample` file is chosen, the synth channel state is loaded (`GBChannelSynth::LoadState`) and the preset name is stored; `.flgroove` files feed the full groove into `GBSynth::LoadState`. Wav files are dispatched further depending on context. 【F:libgroovemachinemobile.so.c†L19589-L19620】
* Saving/export helpers elsewhere also use the same extensions (search for `Engine_SaveFileWithPicker` calls near the 36k–37k region).【F:libgroovemachinemobile.so.c†L36850-L37023】

## UI class map (GBox and controls)

* Core screen controller `GBoxUI` coordinates rendering hooks, file pickers, and focus groups, and exposes a large set of handlers: lifecycle (`UIThreadIdle`, `Suspend`, `UpdateAllValues`), interaction (`TouchBegun/Move/End`, `KeyDown/Up`, `ControlClicked/Released/ValueChanged`), and file dialogs (`OnFileEnum`, `OnFileLoadedWithPicker`, `OnFileSavedWithPicker`).【F:libgroovemachinemobile.so.c†L302-L337】
* Specialized widgets include:
  * `GBoxDropdownList` for enumerated selection; it has its own `DisplaySelector`, `SetItemOnSwipe`, and touch handlers.【F:libgroovemachinemobile.so.c†L182-L271】
  * `GBoxButton` supports color profiles and drag zones for clips/mixer selections, with hover/tap state helpers.【F:libgroovemachinemobile.so.c†L244-L259】
  * `GBoxItemSelector` backs scrolling lists with enable/disable states and cursor management (`MakeSelVisible`, `BoundTopLeftItem`, `MouseWheel`).【F:libgroovemachinemobile.so.c†L272-L289】
  * `CFocusGroup` clusters controls (sliders/buttons/labels) and relays recent parameter changes from the synth/track editors.【F:libgroovemachinemobile.so.c†L186-L205】
  * `CPresetSelControl` wraps preset browsing (next/prev/load) and display rendering.【F:libgroovemachinemobile.so.c†L542-L561】
  * Transport/top bars: `GBoxUI::UpdateTransportControls` orchestrates play/rec/metronome, tempo/out-gain display, CPU meter, and tab buttons that swap screens (Keyboard / Score Edit / Controls / Automation) plus per-part tabs (Kick, Bass, Synth, Lead, Chords, SFX).【F:libgroovemachinemobile.so.c†L300-L338】
* The new `include/ui_controls.h`, `src/ui_controls.c`, and `src/gbox_ui.c` factor these responsibilities into portable C structs and helper functions. `web/gbox_ui.js` mirrors the structures for a canvas/WebGL host, enabling a gradual WASM port while keeping data formats aligned with the native loaders.

## Next steps toward a web port

* The JNI bridge has been isolated into `src/jni_bridge.c` with shared declarations in `include/engine_api.h` to simplify replacing JNI calls with a WebAssembly/WebGL host layer.
* The remaining monolith still contains large UI, synthesis and sequencing subsystems (`CMobileUIControl`, `GBSynth`, `GBSeq`, `CTrackEditor`, `CSongEditor`, etc.). These should be split into logical translation units (e.g., `ui/`, `audio/`, `fileio/`) as they are ported or re‑implemented.
* Custom asset formats of interest:
  * `.flgsynth`: synthesizer preset/state blobs.
  * `.flgsample`: sampler preset/state blobs.
  * `.flgroove`: full groove/song state.
  * `.spectra`: spectral data (referenced during browser filtering; parsing code needs to be lifted when encountered).
  * `.ini`: legacy instrument configs handled alongside `.flgsynth`.
  * `.wav`: raw audio; special directories decide whether the drum or user sample channels receive the files.
* To understand parsing, focus on the loader functions referenced in the browser callbacks (e.g., `GBChannelSynth::LoadState` and `GBSynth::LoadState`) and the save helpers mentioned above; migrating those will define the binary layout for a web parser.
