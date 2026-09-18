#include "list.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_stdinc.h>

static void
update_layout(struct ui_list* list)
{
  // calculate how many items can fit in bounds
  float const capacity =
    SDL_floorf((list->bounds.h + list->gap) / (list->item_height + list->gap));

  // in case the caller didn't give us enough slots
  list->slot_count = SDL_min((int)capacity, list->max_slots);

  struct ui_position pos = { 0 };
  pos.anchor = UI_ANCHOR_TOP_LEFT;

  struct ui_strip strip = { 0 };
  strip.count = list->slot_count;
  strip.gap = list->gap;
  strip.item_height = list->item_height;
  strip.item_width = list->bounds.w;

  ui_layout_column(pos, &list->bounds, strip, list->slots);
}

void
ui_list_init(struct ui_list* list,
             SDL_FRect const* bounds,
             SDL_FRect* slots,
             int max_slots,
             float item_height,
             float gap)
{
  SDL_assert(slots != NULL);
  SDL_assert(max_slots >= 0);
  SDL_assert(item_height > 0);
  SDL_assert(gap >= 0);

  list->bounds = *bounds;
  list->slots = slots;
  list->max_slots = max_slots;
  list->item_height = item_height;
  list->gap = gap;

  list->slot_count = 0;
  list->offset = 0;

  update_layout(list);
}

void
ui_list_resize(struct ui_list* list, SDL_FRect const* bounds)
{
  list->bounds = *bounds;
  update_layout(list);
}

int
ui_list_max_offset(struct ui_list const* list, int count)
{
  count = SDL_max(0, count);
  return SDL_max(0, count - list->slot_count);
}

int
ui_list_offset(struct ui_list const* list, int count)
{
  return SDL_clamp(list->offset, 0, ui_list_max_offset(list, count));
}

void
ui_list_scroll_to(struct ui_list* list, int count, int offset)
{
  list->offset = SDL_clamp(offset, 0, ui_list_max_offset(list, count));
}

void
ui_list_scroll_by(struct ui_list* list, int count, int amount)
{
  ui_list_scroll_to(list, count, ui_list_offset(list, count) + amount);
}

void
ui_list_ensure_visible(struct ui_list* list, int count, int index)
{
  count = SDL_max(0, count);
  if (count == 0) {
    list->offset = 0;
    return;
  }

  index = SDL_clamp(index, 0, count - 1);

  int offset = ui_list_offset(list, count);

  if (index < offset) {
    offset = index;
  } else if (list->slot_count > 0 && index >= offset + list->slot_count) {
    offset = index - list->slot_count + 1;
  }

  ui_list_scroll_to(list, count, offset);
}

int
ui_list_end(struct ui_list const* list, int count)
{
  count = SDL_max(0, count);

  return SDL_min(ui_list_offset(list, count) + list->slot_count, count);
}

static bool
menu_item_enabled(struct ui_list_menu_model model, int index)
{
  return index >= 0 && index < model.count &&
         (model.is_enabled == NULL || model.is_enabled(model.data, index));
}

void
ui_list_menu_sync(struct ui_list_menu* menu, struct ui_list_menu_model model)
{
  int const count = SDL_max(0, model.count);
  int selected = -1;

  if (count > 0) {
    int const start = SDL_clamp(menu->selected, 0, count - 1);
    for (int i = start; i < count; ++i) {
      if (menu_item_enabled(model, i)) {
        selected = i;
        break;
      }
    }
    if (selected < 0) {
      for (int i = start - 1; i >= 0; --i) {
        if (menu_item_enabled(model, i)) {
          selected = i;
          break;
        }
      }
    }
  }

  menu->selected = selected;
  if (selected >= 0) {
    ui_list_ensure_visible(&menu->list, count, selected);
  } else {
    ui_list_scroll_to(&menu->list, count, menu->list.offset);
  }
}

bool
ui_list_menu_move(struct ui_list_menu* menu,
                  struct ui_list_menu_model model,
                  int direction)
{
  SDL_assert(direction == -1 || direction == 1);

  int const previous = menu->selected;
  ui_list_menu_sync(menu, model);
  if (menu->selected >= 0) {
    int i = menu->selected;
    for (int visited = 0; visited < model.count; ++visited) {
      i += direction;
      if (i < 0) {
        i = model.count - 1;
      } else if (i >= model.count) {
        i = 0;
      }
      if (ui_list_menu_select(menu, model, i)) {
        break;
      }
    }
  }

  return menu->selected != previous;
}

bool
ui_list_menu_select(struct ui_list_menu* menu,
                    struct ui_list_menu_model model,
                    int index)
{
  if (!menu_item_enabled(model, index)) {
    return false;
  }

  menu->selected = index;
  ui_list_ensure_visible(&menu->list, model.count, index);
  return true;
}
