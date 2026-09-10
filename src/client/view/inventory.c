#include "inventory.h"

#include <SDL3/SDL_render.h>

#include "game/world.h"

#include "client/view/ribbon.h"

#include "client/palette.h"
#include "client/render.h"
#include "client/ui.h"

static int
held_item_count(struct rl_world const* world)
{
  int len = 0;
  for (int id = 0; id < alist_len(&world->items); ++id) {
    struct rl_item const* item = rl_get_item(world, id);
    if (item->ltype == RL_ITEM_LOCATION_HELD && item->on.actor == RL_ROGUE_ID) {
      ++len;
    }
  }
  return len;
}

static struct rl_text
item_text(struct rl_item const* item, bool selected)
{
  struct rl_item_def const* def = rl_get_item_def(item->itype);
  struct rl_text text = { 0 };

  rl_append_text(&text, &RL_COLOUR_YELLOW[3], selected ? "> " : "  ");
  rl_append_text(&text, NULL, def->name);

  return text;
}

void
rl_init_inv_view(struct rl_inv_view* view,
                 SDL_FRect const* viewport,
                 float line_height)
{
  view->selected = 0;
  ui_list_init(&view->list,
               viewport,
               view->slots,
               SDL_arraysize(view->slots),
               line_height,
               2.0f);
}

void
rl_resize_inv_view(struct rl_inv_view* view, SDL_FRect const* viewport)
{
  ui_list_resize(&view->list, viewport);
}

void
rl_select_inv_view_up(struct rl_inv_view* view, struct rl_world const* world)
{
  int const last = SDL_max(0, held_item_count(world) - 1);
  int const selected = SDL_clamp(view->selected, 0, last);
  rl_select_inv_view_to(view, world, selected - 1);
}

void
rl_select_inv_view_down(struct rl_inv_view* view, struct rl_world const* world)
{
  int const last = SDL_max(0, held_item_count(world) - 1);
  int const selected = SDL_clamp(view->selected, 0, last);
  rl_select_inv_view_to(view, world, selected + 1);
}

void
rl_select_inv_view_to(struct rl_inv_view* view,
                      struct rl_world const* world,
                      int selected)
{
  int const count = held_item_count(world);
  view->selected = SDL_clamp(selected, 0, SDL_max(0, count - 1));
  ui_list_ensure_visible(&view->list, count, view->selected);
}

int
rl_inv_view_selected_item(struct rl_inv_view const* view,
                          struct rl_world const* world)
{
  int index = 0;

  for (int id = 0; id < alist_len(&world->items); ++id) {
    struct rl_item const* item = rl_get_item(world, id);
    if (item->ltype != RL_ITEM_LOCATION_HELD || item->on.actor != RL_ROGUE_ID) {
      continue;
    }

    if (index == view->selected) {
      return item->id;
    }

    ++index;
  }

  return -1;
}

void
rl_inv_view_ribbon(struct rl_inv_view const* view, struct rl_ribbon* ribbon)
{
  (void)view;

  rl_set_current_view(ribbon, "Inventory");
  rl_set_current_mode(ribbon, "Selecting");

  struct rl_text msg = { 0 };
  rl_append_text(&msg, &RL_COLOUR_YELLOW[3], "[WS, E]   ");
  rl_set_ribbon_text(ribbon, RL_RIBBON_RIGHT, &msg);
}

void
rl_draw_inv_view(struct rl_inv_view const* view,
                 SDL_Renderer* renderer,
                 struct rl_font const* font,
                 struct rl_world const* world)
{
  int const len = (int)alist_len(&world->items);
  int const first = ui_list_offset(&view->list, held_item_count(world));
  int skipped = 0;
  int row = 0;

  for (int id = 0; id < len && row < view->list.slot_count; ++id) {
    struct rl_item const* item = rl_get_item(world, id);
    if (item->ltype != RL_ITEM_LOCATION_HELD || item->on.actor != RL_ROGUE_ID) {
      continue;
    }

    if (skipped < first) {
      ++skipped;
      continue;
    }

    struct rl_text const text = item_text(item, first + row == view->selected);

    SDL_FPoint const at = {
      view->slots[row].x,
      view->slots[row].y,
    };
    rl_draw_text(renderer, font, &text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);

    ++row;
  }
}
