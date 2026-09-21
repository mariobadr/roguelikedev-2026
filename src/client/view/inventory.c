#include "inventory.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "game/item_def.h"
#include "game/world.h"

#include "render/graphics.h"
#include "render/palette.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/ribbon.h"
#include "client/text.h"
#include "client/view.h"

#include "ui/list.h"

/** Held items of the same type, shown as one row. */
struct item_group
{
  /** The first held item of this type. */
  handle(rl_item) first;
  /** How many items of this type are held. */
  int count;
};

struct view_state
{
  // the "model" data
  struct rl_world const* world;
  // the UI component
  struct ui_list_menu menu;
  // temporary; these are the rects where we draw the text
  SDL_FRect slots[8];
  // the selected item, waiting to be "pulled"
  handle(rl_item) pending_item;
  // the rows, in the order each item type was first found
  struct item_group groups[RL_ITEM_TYPE_COUNT];
  int group_count;
};

static void
build_groups(struct view_state* s)
{
  handle(rl_actor) const rogue = rl_get_rogue(s->world);
  int const count = rl_count_held_items(s->world, rogue);

  s->group_count = 0;
  for (int i = 0; i < count; ++i) {
    handle(rl_item) const item_handle = rl_find_held_item(s->world, rogue, i);
    struct rl_item const* item = rl_borrow_item(s->world, item_handle);

    int g = 0;
    while (g < s->group_count &&
           rl_borrow_item(s->world, s->groups[g].first)->itype != item->itype) {
      ++g;
    }

    if (g == s->group_count) {
      SDL_assert(s->group_count < (int)SDL_arraysize(s->groups));
      s->groups[g] = (struct item_group){ .first = item_handle, .count = 0 };
      ++s->group_count;
    }
    ++s->groups[g].count;
  }
}

static struct ui_list_menu_model
menu_model(struct view_state const* s)
{
  return (struct ui_list_menu_model){
    .count = s->group_count,
  };
}

static handle(rl_item)
selected_item(struct view_state const* s)
{
  if (s->menu.selected < 0 || s->menu.selected >= s->group_count) {
    return handle_invalid(rl_item);
  }
  return s->groups[s->menu.selected].first;
}

static void
init_view_state(struct view_state* s,
                struct rl_world const* world,
                SDL_FRect const* viewport,
                float line_height)
{
  s->world = world;
  s->menu.selected = -1;
  s->pending_item = handle_invalid(rl_item);

  ui_list_init(&s->menu.list,
               viewport,
               s->slots,
               SDL_arraysize(s->slots),
               line_height,
               2.0f);
  build_groups(s);
  ui_list_menu_sync(&s->menu, menu_model(s));
}

static struct rl_text
item_text(struct rl_item const* item, int count, bool selected)
{
  struct rl_item_def const* def = rl_get_item_def(item->itype);
  SDL_FColor const colour = rl_get_item_gfx(item).fg;
  struct rl_text text = { 0 };

  rl_append_text(&text, &RL_COLOUR_YELLOW[3], selected ? "> " : "  ");
  rl_append_text(&text, &colour, def->name);
  if (count > 1) {
    char suffix[16];
    SDL_snprintf(suffix, sizeof(suffix), " (x%d)", count);
    rl_append_text(&text, &RL_COLOUR_GRAY[5], suffix);
  }

  return text;
}

static void
describe_ribbon(void const* data, struct rl_ribbon_content* content)
{
  struct view_state const* s = (struct view_state const*)data;
  SDL_assert(s != NULL);

  rl_append_text(
    &content->text[RL_RIBBON_LEFT], &RL_COLOUR_CYAN[3], "Inventory");

  struct rl_text* hint = &content->text[RL_RIBBON_RIGHT];
  SDL_FColor const* const colour = &RL_COLOUR_YELLOW[3];
  bool const can_move = menu_model(s).count > 1;
  if (can_move) {
    rl_append_text(hint, colour, "[WS] move");
  }
  struct rl_item const* item = rl_borrow_item(s->world, selected_item(s));
  if (item != NULL) {
    if (can_move) {
      rl_append_text(hint, colour, "   ");
    }
    if (rl_get_item_consumable_def(item->itype) != NULL) {
      rl_append_text(hint, colour, "[E] use");
    } else {
      rl_append_text(hint, colour, "[E] equip");
    }
  }
  if (hint->length > 0) {
    rl_append_text(hint, colour, "   ");
  }
  rl_append_text(hint, colour, "[Esc] back");
}

static bool
update_view(void* data, struct inpt_state const* istate)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  enum rl_action const action = rl_handle_keyboard_input(istate);
  switch (action) {
    case RL_ACTION_MOVE_UP:
      ui_list_menu_move(&s->menu, menu_model(s), -1);
      return true;
    case RL_ACTION_MOVE_DOWN:
      ui_list_menu_move(&s->menu, menu_model(s), +1);
      return true;
    case RL_ACTION_SELECT:
      if (!ui_list_menu_select(&s->menu, menu_model(s), s->menu.selected)) {
        return false;
      }
      s->pending_item = selected_item(s);
      return true;
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

  // Keep rows and selection in bounds after consuming or equipping an item.
  build_groups(s);
  ui_list_menu_sync(&s->menu, menu_model(s));
}

static void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct gfx_tileset const* font)
{
  struct view_state const* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  int const count = s->group_count;
  int const first = ui_list_offset(&s->menu.list, count);

  for (int row = 0; row < s->menu.list.slot_count && first + row < count;
       ++row) {
    struct item_group const* group = &s->groups[first + row];
    struct rl_item const* item = rl_borrow_item(s->world, group->first);

    struct rl_text const text =
      item_text(item, group->count, first + row == s->menu.selected);

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
  view->describe_ribbon = describe_ribbon;
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
