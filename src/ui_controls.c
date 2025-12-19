#include "ui_controls.h"

#include <stddef.h>

void ui_control_init(UIControl *control, int id, UIRect rect) {
  control->id = id;
  control->rect = rect;
  control->value = 0.0f;
  control->visible = 1;
  control->input_enabled = 1;
  control->draw = NULL;
  control->on_value_changed = NULL;
  control->on_trigger = NULL;
  control->user = NULL;
}

void ui_control_set_visible(UIControl *control, int visible) {
  control->visible = visible;
}

void ui_control_set_input(UIControl *control, int enabled) {
  control->input_enabled = enabled;
}

void ui_control_set_value(UIControl *control, float value) {
  control->value = value;
  if (control->on_value_changed) {
    control->on_value_changed(control, value, control->user);
  }
}

int ui_control_contains(const UIControl *control, float x, float y) {
  return x >= control->rect.x && y >= control->rect.y &&
         x <= control->rect.x + control->rect.width &&
         y <= control->rect.y + control->rect.height;
}

void gbox_dropdown_init(GBoxDropdownList *list,
                        int id,
                        const char *title,
                        UIRect rect,
                        UIDropdownItem *items,
                        size_t item_count) {
  ui_control_init(&list->base, id, rect);
  list->title = title;
  list->items = items;
  list->item_count = item_count;
  list->selected_index = item_count ? 0 : -1;
  list->is_open = 0;
}

void gbox_dropdown_set_selected(GBoxDropdownList *list, int index) {
  if (index >= 0 && (size_t)index < list->item_count &&
      list->items[index].enabled) {
    list->selected_index = index;
    ui_control_set_value(&list->base, (float)index);
  }
}

void gbox_dropdown_toggle(GBoxDropdownList *list, int open) {
  list->is_open = open;
}

void gbox_dropdown_touch(GBoxDropdownList *list,
                         UITouchPhase phase,
                         float x,
                         float y) {
  if (!ui_control_contains(&list->base, x, y) || !list->base.input_enabled) {
    return;
  }
  if (phase == UI_TOUCH_BEGIN) {
    gbox_dropdown_toggle(list, !list->is_open);
    if (list->base.on_trigger) {
      list->base.on_trigger(&list->base, list->base.user);
    }
  } else if (phase == UI_TOUCH_END && list->is_open && list->item_count) {
    float item_height = list->base.rect.height /
                        (float)(list->item_count ? list->item_count : 1);
    int index = (int)((y - list->base.rect.y) / item_height);
    gbox_dropdown_set_selected(list, index);
    gbox_dropdown_toggle(list, 0);
  }
}

void gbox_button_init(GBoxButton *button,
                      int id,
                      const char *label,
                      UIRect rect) {
  ui_control_init(&button->base, id, rect);
  button->label = label;
  button->fill = (UIColor){0.2f, 0.2f, 0.2f, 1.0f};
}

void gbox_button_touch(GBoxButton *button,
                       UITouchPhase phase,
                       float x,
                       float y) {
  if (!ui_control_contains(&button->base, x, y) || !button->base.input_enabled) {
    return;
  }
  if (phase == UI_TOUCH_BEGIN && button->base.on_trigger) {
    button->base.on_trigger(&button->base, button->base.user);
  }
}

void gbox_item_selector_init(GBoxItemSelector *selector,
                             int id,
                             UIRect rect,
                             UIDropdownItem *items,
                             size_t item_count) {
  ui_control_init(&selector->base, id, rect);
  selector->items = items;
  selector->item_count = item_count;
  selector->first_visible = 0;
  selector->selected_index = item_count ? 0 : -1;
}

void gbox_item_selector_touch(GBoxItemSelector *selector,
                              UITouchPhase phase,
                              float x,
                              float y) {
  if (!ui_control_contains(&selector->base, x, y) ||
      !selector->base.input_enabled) {
    return;
  }
  if (phase == UI_TOUCH_BEGIN) {
    float item_height = selector->base.rect.height /
                        (float)(selector->item_count ? selector->item_count : 1);
    int index = selector->first_visible +
                (int)((y - selector->base.rect.y) / item_height);
    if (index >= 0 && (size_t)index < selector->item_count &&
        selector->items[index].enabled) {
      selector->selected_index = index;
      ui_control_set_value(&selector->base, (float)index);
    }
  }
}

void gbox_item_selector_scroll(GBoxItemSelector *selector, int delta) {
  int next = selector->first_visible + delta;
  if (next < 0) {
    next = 0;
  }
  if ((size_t)next < selector->item_count) {
    selector->first_visible = next;
  }
}

void gbox_ui_init(GBoxUI *ui, UIRect rect) {
  ui_control_init(&ui->base, 0, rect);
  ui->dropdowns = NULL;
  ui->dropdown_count = 0;
  ui->buttons = NULL;
  ui->button_count = 0;
  ui->item_selectors = NULL;
  ui->item_selector_count = 0;
  ui->focused_group = 0;
  ui->current_screen = 0;
}

void gbox_ui_attach_dropdowns(GBoxUI *ui,
                              GBoxDropdownList *lists,
                              size_t count) {
  ui->dropdowns = lists;
  ui->dropdown_count = count;
}

void gbox_ui_attach_buttons(GBoxUI *ui, GBoxButton *buttons, size_t count) {
  ui->buttons = buttons;
  ui->button_count = count;
}

void gbox_ui_attach_item_selectors(GBoxUI *ui,
                                   GBoxItemSelector *selectors,
                                   size_t count) {
  ui->item_selectors = selectors;
  ui->item_selector_count = count;
}

void gbox_ui_touch(GBoxUI *ui, UITouchPhase phase, float x, float y) {
  for (size_t i = 0; i < ui->dropdown_count; ++i) {
    gbox_dropdown_touch(&ui->dropdowns[i], phase, x, y);
  }
  for (size_t i = 0; i < ui->button_count; ++i) {
    gbox_button_touch(&ui->buttons[i], phase, x, y);
  }
  for (size_t i = 0; i < ui->item_selector_count; ++i) {
    gbox_item_selector_touch(&ui->item_selectors[i], phase, x, y);
  }
}
