#ifndef UI_CONTROLS_H
#define UI_CONTROLS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  UI_TOUCH_BEGIN = 0,
  UI_TOUCH_MOVE = 1,
  UI_TOUCH_END = 2
} UITouchPhase;

typedef struct {
  float r, g, b, a;
} UIColor;

typedef struct {
  float x, y, width, height;
} UIRect;

typedef struct UIControl UIControl;

typedef void (*UIDrawFn)(UIControl *control, void *user);
typedef void (*UIValueChangedFn)(UIControl *control, float value, void *user);
typedef void (*UITriggeredFn)(UIControl *control, void *user);

struct UIControl {
  int id;
  UIRect rect;
  float value;
  int visible;
  int input_enabled;
  UIDrawFn draw;
  UIValueChangedFn on_value_changed;
  UITriggeredFn on_trigger;
  void *user;
};

typedef struct {
  const char *label;
  int enabled;
  void *payload;
} UIDropdownItem;

typedef struct {
  UIControl base;
  const char *title;
  UIDropdownItem *items;
  size_t item_count;
  int selected_index;
  int is_open;
} GBoxDropdownList;

typedef struct {
  UIControl base;
  const char *label;
  UIColor fill;
} GBoxButton;

typedef struct {
  UIControl base;
  UIDropdownItem *items;
  size_t item_count;
  int first_visible;
  int selected_index;
} GBoxItemSelector;

typedef struct {
  UIControl base;
  GBoxDropdownList *dropdowns;
  size_t dropdown_count;
  GBoxButton *buttons;
  size_t button_count;
  GBoxItemSelector *item_selectors;
  size_t item_selector_count;
  int focused_group;
  int current_screen;
} GBoxUI;

typedef struct {
  GBoxButton groove_icon;
  GBoxButton back;
  GBoxButton play;
  GBoxButton record;
  GBoxButton metronome;
  float tempo_bpm;
  float out_gain_db;
  float cpu_usage;
  GBoxButton mode_tabs[4];   /* Keyboard, Score Edit, Controls, Automation */
  GBoxButton part_tabs[6];   /* Kick, Bass, Synth, Lead, Chords, SFX */
} GBoxTransportBar;

void ui_control_init(UIControl *control, int id, UIRect rect);
void ui_control_set_visible(UIControl *control, int visible);
void ui_control_set_input(UIControl *control, int enabled);
void ui_control_set_value(UIControl *control, float value);
int ui_control_contains(const UIControl *control, float x, float y);

void gbox_dropdown_init(GBoxDropdownList *list,
                        int id,
                        const char *title,
                        UIRect rect,
                        UIDropdownItem *items,
                        size_t item_count);
void gbox_dropdown_set_selected(GBoxDropdownList *list, int index);
void gbox_dropdown_toggle(GBoxDropdownList *list, int open);
void gbox_dropdown_touch(GBoxDropdownList *list,
                         UITouchPhase phase,
                         float x,
                         float y);

void gbox_button_init(GBoxButton *button,
                      int id,
                      const char *label,
                      UIRect rect);
void gbox_button_touch(GBoxButton *button,
                       UITouchPhase phase,
                       float x,
                       float y);

void gbox_item_selector_init(GBoxItemSelector *selector,
                             int id,
                             UIRect rect,
                             UIDropdownItem *items,
                             size_t item_count);
void gbox_item_selector_touch(GBoxItemSelector *selector,
                              UITouchPhase phase,
                              float x,
                              float y);
void gbox_item_selector_scroll(GBoxItemSelector *selector, int delta);

void gbox_ui_init(GBoxUI *ui, UIRect rect);
void gbox_ui_attach_dropdowns(GBoxUI *ui,
                              GBoxDropdownList *lists,
                              size_t count);
void gbox_ui_attach_buttons(GBoxUI *ui, GBoxButton *buttons, size_t count);
void gbox_ui_attach_item_selectors(GBoxUI *ui,
                                   GBoxItemSelector *selectors,
                                   size_t count);
void gbox_ui_touch(GBoxUI *ui, UITouchPhase phase, float x, float y);
void gbox_ui_transport_init(GBoxTransportBar *bar,
                            float tempo_bpm,
                            float out_gain_db);
void gbox_ui_transport_set_tempo(GBoxTransportBar *bar, float tempo_bpm);
void gbox_ui_transport_set_gain(GBoxTransportBar *bar, float gain_db);
void gbox_ui_transport_set_cpu(GBoxTransportBar *bar, float cpu_usage);

#ifdef __cplusplus
}
#endif

#endif /* UI_CONTROLS_H */
