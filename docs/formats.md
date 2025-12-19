# Groove Machine Mobile file formats (reader notes)

Derived from the decompiled native code (`libgroovemachinemobile.so.c`) and matching the loaders used in the Android build.

## `.flgsynth` / `.flgsample` (GBChannelSynth::LoadState)

* Magic: `0x43686E32` (`"Chn2"`) with minor revisions accepted up to `0x43686E34`.
* Layout (little‑endian):
  * `u32 magic`
  * `char[512] sampleAPath` (null‑terminated)
  * `char[512] sampleBPath`
  * `float params[74]` for `"Chn2"`; 79 floats for later revisions (the native code uses `74 + (magic != "Chn2" ? 5 : 0)`).
  * `u32 generatorStateLen`
  * `u8[generatorStateLen] generatorState` (sampler/generator blob, passed to `CSmpSynthGenerator::SetState`).
* Notable behaviours:
  * The two sample paths are resolved against install or local folders (`GetInstallFolder` / `GetLocalAppFolder`), then loaded via `LoadDrumSampleFromLib`.
  * Each parameter float also seeds automation init values via `GBSeq::GetAutomationInitValuePtr`.

## `.flgroove` (GBSeq::SeqLoadState)

* Magic: `0x53657133` (`"Seq3"`) or `0x53657134` (`"Seq4"`).
* Layout (per channel, little‑endian):
  * `u16 eventCount`
  * `u32 patternLenTicks` (duration for that channel/clip bank)
  * `eventCount` records, each 17 bytes:
    * `u32 tick`
    * `u8 eventType`
    * `f32 value`
    * `f64 payload` (varies by event type; stored verbatim)
* The native loader locks each channel’s `CEventBuffer`, pushes events, and unlocks again. Unknown bytes are tolerated; later versions may extend the payload, so keep raw bytes when parsing.

## `.spectra`

* Only enumerated in file pickers; parsing is not present in the recovered blob. Expect a custom spectral table; format still unknown.

## JS reference parser

* `web/formats.js` implements `parseFlgsynth`/`parseFlgsample` and `parseFlgroove`, extracting headers, sample paths, parameter arrays, automation seeds, and raw event payloads to remain compatible with the native structures.
