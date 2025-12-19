/**
 * Parsers for Groove Machine Mobile formats (.flgsynth, .flgsample, .flgroove).
 * These are based on the native loaders found in libgroovemachinemobile.so:
 * - GBChannelSynth::LoadState for .flgsynth / .flgsample
 * - GBSeq::SeqLoadState for .flgroove
 *
 * The goal is to extract useful metadata and raw parameter blocks so the web
 * UI can become data-compatible with the original app.
 */

const textDecoder = new TextDecoder("utf-8");

function readCString(view, offset, maxLen) {
  const bytes = [];
  for (let i = 0; i < maxLen; i++) {
    const b = view.getUint8(offset + i);
    if (b === 0) break;
    bytes.push(b);
  }
  return textDecoder.decode(new Uint8Array(bytes));
}

function readFloatArray(view, offset, count) {
  const out = [];
  for (let i = 0; i < count; i++) {
    out.push(view.getFloat32(offset + i * 4, true));
  }
  return out;
}

/**
 * Parse .flgsynth / .flgsample files (GBChannelSynth::LoadState).
 *
 * Layout (little-endian):
 *   u32 magic: 0x43686E32 ('Chn2') or +1/+2 for later revisions
 *   char[512] sampleAPath
 *   char[512] sampleBPath
 *   params: float32[74] (or 79 if magic != 0x43686E32)
 *   u32 generatorStateLen
 *   u8[generatorStateLen] generatorState (sampler-specific payload)
 */
export function parseFlgsynth(buffer) {
  const view = new DataView(buffer);
  if (view.byteLength < 4) {
    throw new Error("Buffer too small");
  }
  const magic = view.getUint32(0, true);
  // Accept 'Chn2' and nearby versions (+1/+2)
  if (magic < 0x43686e32 || magic > 0x43686e34) {
    throw new Error("Unrecognized flgsynth/flgsample magic");
  }

  const sampleA = readCString(view, 4, 512);
  const sampleB = readCString(view, 516, 512);

  const paramCount = magic === 0x43686e32 ? 74 : 79;
  const paramsOffset = 4 + 512 * 2;
  const params = readFloatArray(view, paramsOffset, paramCount);

  const stateLenOffset = paramsOffset + paramCount * 4;
  if (stateLenOffset + 4 > view.byteLength) {
    return { magic, sampleA, sampleB, params, generatorState: new Uint8Array(), version: magic };
  }
  const stateLen = view.getUint32(stateLenOffset, true);
  const stateStart = stateLenOffset + 4;
  const stateEnd = Math.min(view.byteLength, stateStart + stateLen);
  const generatorState = new Uint8Array(buffer.slice(stateStart, stateEnd));

  return { magic, sampleA, sampleB, params, generatorState, version: magic };
}

/**
 * Parse .flgroove files (GBSeq::SeqLoadState).
 *
 * Layout (little-endian, coarse):
 *   u32 magic: 0x53657133 ('Seq3') or 0x53657134 ('Seq4')
 *   For each channel:
 *     u16 eventCount (n)
 *     u32 patternLenTicks
 *     n records of 17 bytes each:
 *       u32 tick
 *       u8  eventType
 *       f32 value
 *       f64 payload (often automation value / clip info)
 *
 * Unknown/extra bytes are preserved verbatim in each record entry so the
 * parser is robust to minor format shifts across versions.
 */
export function parseFlgroove(buffer) {
  const view = new DataView(buffer);
  if (view.byteLength < 4) {
    throw new Error("Buffer too small");
  }
  const magic = view.getUint32(0, true);
  if (magic !== 0x53657133 && magic !== 0x53657134) {
    throw new Error("Unrecognized flgroove magic");
  }
  const channels = [];
  let offset = 4;
  let channelIndex = 0;

  while (offset + 6 <= view.byteLength) {
    const eventCount = view.getUint16(offset, true);
    const patternLen = view.getUint32(offset + 2, true);
    offset += 6;
    const events = [];
    for (let i = 0; i < eventCount; i++) {
      if (offset + 17 > view.byteLength) break;
      const tick = view.getUint32(offset, true);
      const eventType = view.getUint8(offset + 4);
      const value = view.getFloat32(offset + 5, true);
      const payload = view.getFloat64(offset + 9, true);
      const raw = new Uint8Array(buffer.slice(offset, offset + 17));
      events.push({ tick, eventType, value, payload, raw });
      offset += 17;
    }
    channels.push({ index: channelIndex++, patternLen, events });
  }

  return { magic, channels };
}

export function parseFlgsample(buffer) {
  // Same layout as flgsynth; keep a separate entry for clarity.
  return parseFlgsynth(buffer);
}
