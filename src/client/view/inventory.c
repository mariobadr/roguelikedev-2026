#include "inventory.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "game/world.h"

#include "client/view/ribbon.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/ui.h"
#include "client/view.h"

#include "ui/list.h"

struct view_state
{
  struct rl_world const* world;

  struct ui_list list;
  /** Selected item in the filtered inventory. */
  int selected;
  SDL_FRect slots[8]; // temporary; these are the rects where we draw the text
};

static void
init_view_state(struct view_state* s,
                struct rl_world const* world,
                SDL_FRect const* viewport,
                float line_height)
{
  s->world = world;
  s->selected = 0;

  ui_list_init(
    &s->list, viewport, s->slots, SDL_arraysize(s->slots), line_height, 2.0f);
}

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
rl_select_inv_view_to(struct view_state* view,
                      int selected)
{
  int const count = held_item_count(view->world);
  view->selected = SDL_clamp(selected, 0, SDL_max(0, count - 1));
  ui_list_ensure_visible(&view->list, count, view->selected);
}

void
rl_select_inv_view_up(struct view_state* view)
{
  int const last = SDL_max(0, held_item_count(view->world) - 1);
  int const selected = SDL_clamp(view->selected, 0, last);
  rl_select_inv_view_to(view, selected - 1);
}

void
rl_select_inv_view_down(struct view_state* view)
{
  int const last = SDL_max(0, held_item_count(view->world) - 1);
  int const selected = SDL_clamp(view->selected, 0, last);
  rl_select_inv_view_to(view, selected + 1);
}

int
rl_inv_view_selected_item(struct view_state const* view)
{
  int index = 0;

  for (int id = 0; id < alist_len(&view->world->items); ++id) {
    struct rl_item const* item = rl_get_item(view->world, id);
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

static bool
can_interact(void)
{
  return true;
}

static void
update_ribbon(void const* data, struct rl_ribbon* ribbon)
{
  (void)data;

  rl_set_current_view(ribbon, "Inventory");
  rl_set_current_mode(ribbon, "Selecting");

  struct rl_text msg = { 0 };
  rl_append_text(&msg, &RL_COLOUR_YELLOW[3], "[WS, E]");
  rl_set_ribbon_text(ribbon, RL_RIBBON_RIGHT, &msg);
}

static bool
update_view(void* data, struct inpt_state const* istate, struct rl_command* out)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  enum rl_action const action = rl_handle_keyboard_input(istate);
  switch (action) {
    case RL_ACTION_MOVE_UP:
      rl_select_inv_view_up(s);
      return true;
    case RL_ACTION_MOVE_DOWN:
      rl_select_inv_view_down(s);
      return true;
    case RL_ACTION_SELECT: {
      int const item_id = rl_inv_view_selected_item(s);
      if (item_id < 0) {
        return false;
      }

      out->actor = RL_ROGUE_ID;
      out->type = RL_COMMAND_USE_ITEM;
      out->target = item_id;

      return true;
    }
    default:
      break;
  }

  return false;
}

static void
prepare_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  // Keep selection in bounds after consuming an item.
  rl_select_inv_view_to(s, s->selected);
}

static void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct rl_font const* font)
{
  struct view_state const* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  int const len = (int)alist_len(&s->world->items);
  int const first = ui_list_offset(&s->list, held_item_count(s->world));
  int skipped = 0;
  int row = 0;

  for (int id = 0; id < len && row < s->list.slot_count; ++id) {
    struct rl_item const* item = rl_get_item(s->world, id);
    if (item->ltype != RL_ITEM_LOCATION_HELD || item->on.actor != RL_ROGUE_ID) {
      continue;
    }

    if (skipped < first) {
      ++skipped;
      continue;
    }

    struct rl_text const text = item_text(item, first + row == s->selected);

    SDL_FPoint const at = {
      s->slots[row].x,
      s->slots[row].y,
    };
    rl_draw_text(renderer, font, &text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);

    ++row;
  }
}

bool
rl_alloc_inv_view(struct rl_view* view,
                  struct rl_world const* world,
                  SDL_FRect const* viewport,
                  float line_height)
{
  view->state = SDL_calloc(1, sizeof(struct view_state));
  if (view->state == NULL) {
    return false;
  }

  init_view_state(view->state, world, viewport, line_height);

  view->can_interact = can_interact;
  view->free = SDL_free;
  view->update_ribbon = update_ribbon;
  view->update = update_view;
  view->prepare = prepare_view;
  view->render = render_view;

  return true;
}