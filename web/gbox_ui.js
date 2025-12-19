/**
 * Minimal canvas-based reimplementation of key Groove Machine Mobile UI pieces.
 * The goal is to mirror the C structs in ui_controls.h so a WebAssembly core
 * can drive the same data structures and remain file-format compatible.
 */

export class UIControl {
  constructor(id, rect) {
    this.id = id;
    this.rect = rect;
    this.value = 0;
    this.visible = true;
    this.inputEnabled = true;
    this.onValueChanged = null;
    this.onTrigger = null;
  }

  contains(x, y) {
    return (
      x >= this.rect.x &&
      y >= this.rect.y &&
      x <= this.rect.x + this.rect.width &&
      y <= this.rect.y + this.rect.height
    );
  }
}

export class Dropdown extends UIControl {
  constructor(id, title, rect, items = []) {
    super(id, rect);
    this.title = title;
    this.items = items;
    this.selectedIndex = items.length ? 0 : -1;
    this.isOpen = false;
  }

  toggle(open) {
    this.isOpen = open ?? !this.isOpen;
  }

  select(index) {
    if (index >= 0 && index < this.items.length && this.items[index].enabled) {
      this.selectedIndex = index;
      this.value = index;
      this.onValueChanged?.(this.value);
    }
  }

  touch(phase, x, y) {
    if (!this.inputEnabled || !this.contains(x, y)) return;
    if (phase === "begin") {
      this.toggle();
      this.onTrigger?.();
    } else if (phase === "end" && this.isOpen) {
      const itemHeight = this.rect.height / Math.max(1, this.items.length);
      const idx = Math.floor((y - this.rect.y) / itemHeight);
      this.select(idx);
      this.toggle(false);
    }
  }

  draw(ctx) {
    if (!this.visible) return;
    ctx.save();
    ctx.strokeStyle = "#6cf";
    ctx.strokeRect(this.rect.x, this.rect.y, this.rect.width, this.rect.height);
    ctx.fillStyle = "#0a0c10";
    ctx.fillRect(this.rect.x, this.rect.y, this.rect.width, this.rect.height);
    ctx.fillStyle = "#fff";
    ctx.font = "12px sans-serif";
    const label =
      this.selectedIndex >= 0
        ? this.items[this.selectedIndex].label
        : this.title;
    ctx.fillText(
      label,
      this.rect.x + 6,
      this.rect.y + this.rect.height / 2 + 4
    );
    ctx.restore();
  }
}

export class Button extends UIControl {
  constructor(id, label, rect) {
    super(id, rect);
    this.label = label;
  }

  touch(phase, x, y) {
    if (phase === "begin" && this.contains(x, y) && this.inputEnabled) {
      this.onTrigger?.();
    }
  }

  draw(ctx) {
    if (!this.visible) return;
    ctx.save();
    ctx.fillStyle = "#223";
    ctx.fillRect(this.rect.x, this.rect.y, this.rect.width, this.rect.height);
    ctx.strokeStyle = "#6cf";
    ctx.strokeRect(this.rect.x, this.rect.y, this.rect.width, this.rect.height);
    ctx.fillStyle = "#fff";
    ctx.font = "12px sans-serif";
    ctx.fillText(
      this.label,
      this.rect.x + 6,
      this.rect.y + this.rect.height / 2 + 4
    );
    ctx.restore();
  }
}

export class ItemSelector extends UIControl {
  constructor(id, rect, items = []) {
    super(id, rect);
    this.items = items;
    this.firstVisible = 0;
    this.selectedIndex = items.length ? 0 : -1;
  }

  select(index) {
    if (index >= 0 && index < this.items.length && this.items[index].enabled) {
      this.selectedIndex = index;
      this.value = index;
      this.onValueChanged?.(this.value);
    }
  }

  scroll(delta) {
    this.firstVisible = Math.max(
      0,
      Math.min(this.firstVisible + delta, this.items.length - 1)
    );
  }

  touch(phase, x, y) {
    if (phase !== "begin" || !this.contains(x, y) || !this.inputEnabled) return;
    const itemHeight = this.rect.height / Math.max(1, this.items.length);
    const idx = this.firstVisible + Math.floor((y - this.rect.y) / itemHeight);
    this.select(idx);
  }

  draw(ctx) {
    if (!this.visible) return;
    ctx.save();
    ctx.fillStyle = "#0a0c10";
    ctx.fillRect(this.rect.x, this.rect.y, this.rect.width, this.rect.height);
    ctx.strokeStyle = "#6cf";
    ctx.strokeRect(this.rect.x, this.rect.y, this.rect.width, this.rect.height);
    ctx.fillStyle = "#fff";
    ctx.font = "12px sans-serif";
    const itemHeight = this.rect.height / Math.max(1, this.items.length);
    this.items.forEach((item, i) => {
      const y = this.rect.y + itemHeight * i;
      if (!item.enabled) ctx.globalAlpha = 0.35;
      if (i === this.selectedIndex) {
        ctx.fillStyle = "#335";
        ctx.fillRect(this.rect.x, y, this.rect.width, itemHeight);
        ctx.fillStyle = "#fff";
      }
      ctx.fillText(item.label, this.rect.x + 6, y + itemHeight / 2 + 4);
      ctx.globalAlpha = 1;
    });
    ctx.restore();
  }
}

export class GBoxUI extends UIControl {
  constructor(rect) {
    super(0, rect);
    this.dropdownLists = [];
    this.buttons = [];
    this.itemSelectors = [];
    this.currentScreen = 0;
  }

  touch(phase, x, y) {
    this.dropdownLists.forEach((d) => d.touch(phase, x, y));
    this.buttons.forEach((b) => b.touch(phase, x, y));
    this.itemSelectors.forEach((s) => s.touch(phase, x, y));
  }

  draw(ctx) {
    ctx.save();
    ctx.fillStyle = "#090b10";
    ctx.fillRect(this.rect.x, this.rect.y, this.rect.width, this.rect.height);
    ctx.restore();
    this.dropdownLists.forEach((d) => d.draw(ctx));
    this.buttons.forEach((b) => b.draw(ctx));
    this.itemSelectors.forEach((s) => s.draw(ctx));
  }
}

/**
 * Build a demo UI that mirrors the native file filters:
 * flgsynth/flgsample/flgroove/spectra/wav
 */
export function createDemoGBoxUI(canvas) {
  const ui = new GBoxUI({ x: 0, y: 0, width: canvas.width, height: canvas.height });
  const extItems = [
    { label: ".flgsynth", enabled: true },
    { label: ".flgsample", enabled: true },
    { label: ".flgroove", enabled: true },
    { label: ".spectra", enabled: true },
    { label: ".wav", enabled: true },
  ];
  const dropdown = new Dropdown(1, "Choose type", { x: 20, y: 20, width: 160, height: 32 }, extItems);
  ui.dropdownLists.push(dropdown);

  const loadBtn = new Button(2, "Load Preset", { x: 200, y: 20, width: 120, height: 32 });
  const saveBtn = new Button(3, "Save Groove", { x: 340, y: 20, width: 120, height: 32 });
  ui.buttons.push(loadBtn, saveBtn);

  const selector = new ItemSelector(4, { x: 20, y: 80, width: canvas.width - 40, height: canvas.height - 120 }, [
    { label: "Preset 1", enabled: true },
    { label: "Preset 2", enabled: true },
    { label: "Groove Demo", enabled: true },
    { label: "Drum Kit", enabled: true },
  ]);
  ui.itemSelectors.push(selector);

  return ui;
}

/**
 * Minimal boot routine to emulate the Android Activity -> GLSurfaceView chain:
 * - creates the UI
 * - sets up pointer events
 * - runs a continuous render loop (akin to RENDERMODE_CONTINUOUSLY)
 * - posts a 100ms UI-thread idle tick (like the Java handler in ILGLSurfaceView)
 */
export function bootDemoApp(canvas) {
  const dpr = window.devicePixelRatio || 1;
  const cssWidth = canvas.clientWidth;
  const cssHeight = canvas.clientHeight;
  canvas.width = Math.round(cssWidth * dpr);
  canvas.height = Math.round(cssHeight * dpr);
  const ctx = canvas.getContext("2d");
  ctx.scale(dpr, dpr);

  const ui = createDemoGBoxUI({ width: cssWidth, height: cssHeight });

  const toPhase = (type) => {
    if (type === "pointerdown") return "begin";
    if (type === "pointermove") return "move";
    if (type === "pointerup" || type === "pointercancel") return "end";
    return null;
  };

  const handlePointer = (evt) => {
    const rect = canvas.getBoundingClientRect();
    const x = (evt.clientX - rect.left) * (canvas.width / dpr / rect.width);
    const y = (evt.clientY - rect.top) * (canvas.height / dpr / rect.height);
    const phase = toPhase(evt.type);
    if (!phase) return;
    ui.touch(phase, x, y);
  };

  ["pointerdown", "pointermove", "pointerup", "pointercancel"].forEach((t) =>
    canvas.addEventListener(t, handlePointer)
  );

  // UI thread idle tick every ~100ms
  const uiIdleInterval = setInterval(() => {
    // placeholder for future native-like UI thread work
  }, 100);

  let running = true;
  const render = () => {
    if (!running) return;
    ctx.clearRect(0, 0, cssWidth, cssHeight);
    ui.draw(ctx);
    requestAnimationFrame(render);
  };
  render();

  const teardown = () => {
    running = false;
    clearInterval(uiIdleInterval);
    ["pointerdown", "pointermove", "pointerup", "pointercancel"].forEach((t) =>
      canvas.removeEventListener(t, handlePointer)
    );
  };

  return { ui, teardown };
}
