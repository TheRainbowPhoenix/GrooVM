#include "ui_controls.h"

#include <stdio.h>
#include <string.h>

typedef enum {
  GBOX_FILE_UNKNOWN = 0,
  GBOX_FILE_SYNTH,
  GBOX_FILE_SAMPLE,
  GBOX_FILE_SPECTRA,
  GBOX_FILE_WAV,
  GBOX_FILE_GROOVE
} GBoxFileType;

typedef struct {
  const char *name;
  GBoxFileType type;
} GBoxExtensionRule;

static const GBoxExtensionRule kGBoxExtensionRules[] = {
    {"flgsynth", GBOX_FILE_SYNTH}, {"ini", GBOX_FILE_SYNTH},
    {"flgsample", GBOX_FILE_SAMPLE}, {"spectra", GBOX_FILE_SPECTRA},
    {"wav", GBOX_FILE_WAV},          {"flgroove", GBOX_FILE_GROOVE}};

static GBoxFileType gbox_classify_extension(const char *ext) {
  for (size_t i = 0; i < sizeof(kGBoxExtensionRules) /
                             sizeof(kGBoxExtensionRules[0]);
       ++i) {
    if (strcmp(ext, kGBoxExtensionRules[i].name) == 0) {
      return kGBoxExtensionRules[i].type;
    }
  }
  return GBOX_FILE_UNKNOWN;
}

typedef struct {
  GBoxFileType type;
  const char *path;
  const char *display_name;
} GBoxFileRecord;

typedef struct {
  GBoxFileRecord *records;
  size_t count;
  size_t capacity;
} GBoxFileBuffer;

void gbox_file_buffer_add(GBoxFileBuffer *buffer,
                          GBoxFileType type,
                          const char *path,
                          const char *display_name) {
  if (buffer->count < buffer->capacity) {
    buffer->records[buffer->count++] =
        (GBoxFileRecord){type, path, display_name};
  }
}

static const char *strip_dirname(const char *path) {
  const char *slash = strrchr(path, '/');
  return slash ? slash + 1 : path;
}

void gbox_ui_on_file_enum(GBoxUI *ui,
                          const char *path,
                          const char *ext,
                          int from_user_folder,
                          GBoxFileBuffer *dest_synth,
                          GBoxFileBuffer *dest_sample,
                          GBoxFileBuffer *dest_spectra,
                          GBoxFileBuffer *dest_groove,
                          GBoxFileBuffer *dest_wav) {
  (void)ui;
  GBoxFileType type = gbox_classify_extension(ext);
  const char *name = strip_dirname(path);
  switch (type) {
  case GBOX_FILE_SYNTH:
    gbox_file_buffer_add(dest_synth, type, path, name);
    break;
  case GBOX_FILE_SAMPLE:
    gbox_file_buffer_add(dest_sample, type, path, name);
    break;
  case GBOX_FILE_SPECTRA:
    gbox_file_buffer_add(dest_spectra, type, path, name);
    break;
  case GBOX_FILE_GROOVE:
    gbox_file_buffer_add(dest_groove, type, path, name);
    break;
  case GBOX_FILE_WAV:
    if (from_user_folder) {
      gbox_file_buffer_add(dest_sample, type, path, name);
    } else {
      gbox_file_buffer_add(dest_wav, type, path, name);
    }
    break;
  default:
    break;
  }
}

typedef struct {
  char synth_name[256];
  char sample_name[256];
  char groove_name[256];
} GBoxLoadedState;

void gbox_ui_on_file_loaded_with_picker(GBoxUI *ui,
                                        const char *path,
                                        const char *ext,
                                        GBoxLoadedState *state) {
  (void)ui;
  const char *name = strip_dirname(path);
  GBoxFileType type = gbox_classify_extension(ext);
  switch (type) {
  case GBOX_FILE_SYNTH:
    strncpy(state->synth_name, name, sizeof(state->synth_name) - 1);
    state->synth_name[sizeof(state->synth_name) - 1] = '\0';
    ui_control_set_value(&ui->base, 1.0f);
    break;
  case GBOX_FILE_SAMPLE:
    strncpy(state->sample_name, name, sizeof(state->sample_name) - 1);
    state->sample_name[sizeof(state->sample_name) - 1] = '\0';
    ui_control_set_value(&ui->base, 2.0f);
    break;
  case GBOX_FILE_GROOVE:
    strncpy(state->groove_name, name, sizeof(state->groove_name) - 1);
    state->groove_name[sizeof(state->groove_name) - 1] = '\0';
    ui_control_set_value(&ui->base, 3.0f);
    break;
  default:
    break;
  }
}

void gbox_ui_update_all_values(GBoxUI *ui) {
  // Placeholder for syncing nested controls; this mirrors the numerous
  // UpdateAllValues calls in the original blob.
  if (ui->dropdowns) {
    for (size_t i = 0; i < ui->dropdown_count; ++i) {
      ui_control_set_value(&ui->dropdowns[i].base,
                           (float)ui->dropdowns[i].selected_index);
    }
  }
}

void gbox_ui_display_screen(GBoxUI *ui, int screen_index) {
  ui->current_screen = screen_index;
}

static void init_tab(GBoxButton *btn, int id, const char *label, float x) {
  gbox_button_init(btn, id, label, (UIRect){x, 0.0f, 100.0f, 28.0f});
}

void gbox_ui_transport_init(GBoxTransportBar *bar,
                            float tempo_bpm,
                            float out_gain_db) {
  gbox_button_init(&bar->groove_icon, 100, "Groove", (UIRect){0, 0, 72, 32});
  gbox_button_init(&bar->back, 101, "Back", (UIRect){76, 0, 64, 32});
  gbox_button_init(&bar->play, 102, "Play", (UIRect){144, 0, 64, 32});
  gbox_button_init(&bar->record, 103, "Rec", (UIRect){212, 0, 64, 32});
  gbox_button_init(&bar->metronome, 104, "Metro", (UIRect){280, 0, 72, 32});
  bar->tempo_bpm = tempo_bpm;
  bar->out_gain_db = out_gain_db;
  bar->cpu_usage = 0.0f;

  const char *mode_labels[4] = {"Keyboard", "Score Edit", "Controls", "Automation"};
  for (int i = 0; i < 4; ++i) {
    init_tab(&bar->mode_tabs[i], 200 + i, mode_labels[i], 360.0f + i * 96.0f);
  }

  const char *part_labels[6] = {"Kick", "Bass", "Synth", "Lead", "Chords", "SFX"};
  for (int i = 0; i < 6; ++i) {
    init_tab(&bar->part_tabs[i], 300 + i, part_labels[i], 360.0f + i * 80.0f);
  }
}

void gbox_ui_transport_set_tempo(GBoxTransportBar *bar, float tempo_bpm) {
  bar->tempo_bpm = tempo_bpm;
}

void gbox_ui_transport_set_gain(GBoxTransportBar *bar, float gain_db) {
  bar->out_gain_db = gain_db;
}

void gbox_ui_transport_set_cpu(GBoxTransportBar *bar, float cpu_usage) {
  bar->cpu_usage = cpu_usage;
}
