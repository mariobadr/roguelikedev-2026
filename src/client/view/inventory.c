#include "inventory.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "game/item_def.h"
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
  // the "model" data
  struct rl_world const* world;
  // the UI component
  struct ui_list list;
  // temporary; these are the rects where we draw the text
  SDL_FRect slots[8];
  // selected item index in the filtered inventory
  int selected;
  // the selected item, waiting to be "pulled"
  handle(rl_item) pending_item;
};

static void
init_view_state(struct view_state* s,
                struct rl_world const* world,
                SDL_FRect const* viewport,
                float line_height)
{
  s->world = world;
  s->selected = 0;
  s->pending_item = handle_invalid(rl_item);

  ui_list_init(
    &s->list, viewport, s->slots, SDL_arraysize(s->slots), line_height, 2.0f);
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

static void
scroll_to(struct view_state* view, int selected)
{
  int const count = rl_count_held_items(view->world, rl_get_rogue(view->world));
  view->selected = SDL_clamp(selected, 0, SDL_max(0, count - 1));
  ui_list_ensure_visible(&view->list, count, view->selected);
}

static void
scroll_up(struct view_state* view)
{
  int const count = rl_count_held_items(view->world, rl_get_rogue(view->world));
  int const last = SDL_max(0, count - 1);
  int const selected = SDL_clamp(view->selected, 0, last);
  scroll_to(view, selected - 1);
}

static void
scroll_down(struct view_state* view)
{
  int const count = rl_count_held_items(view->world, rl_get_rogue(view->world));
  int const last = SDL_max(0, count - 1);
  int const selected = SDL_clamp(view->selected, 0, last);
  scroll_to(view, selected + 1);
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
update_view(void* data, struct inpt_state const* istate)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  enum rl_action const action = rl_handle_keyboard_input(istate);
  switch (action) {
    case RL_ACTION_MOVE_UP:
      scroll_up(s);
      return true;
    case RL_ACTION_MOVE_DOWN:
      scroll_down(s);
      return true;
    case RL_ACTION_SELECT:
      s->pending_item =
        rl_find_held_item(s->world, rl_get_rogue(s->world), s->selected);
      return handle_is_nonnull(s->pending_item);
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
  scroll_to(s, s->selected);
}

static void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct rl_font const* font)
{
  struct view_state const* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  handle(rl_actor) const rogue = rl_get_rogue(s->world);
  int const count = rl_count_held_items(s->world, rogue);
  int const first = ui_list_offset(&s->list, count);

  for (int row = 0; row < s->list.slot_count && first + row < count; ++row) {
    handle(rl_item) const item_handle =
      rl_find_held_item(s->world, rogue, first + row);
    struct rl_item const* item = rl_borrow_item(s->world, item_handle);
    if (item == NULL) {
      break;
    }

    struct rl_text const text = item_text(item, first + row == s->selected);

    SDL_FPoint const at = {
      s->slots[row].x,
      s->slots[row].y,
    };
    rl_draw_text(renderer, font, &text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);
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

  view->free = SDL_free;
  view->update_ribbon = update_ribbon;
  view->update = update_view;
  view->prepare = prepare_view;
  view->render = render_view;

  return true;
}

handle(rl_item)
rl_inv_view_take_selection(struct rl_view* view)
{
  struct view_state* s = view->state;
  SDL_assert(s != NULL);

  handle(rl_item) const item = s->pending_item;
  s->pending_item = handle_invalid(rl_item);

  return item;
}
