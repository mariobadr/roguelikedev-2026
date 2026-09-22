#include "inventory.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "container/alist.h"

#include "game/combat.h"
#include "game/item.h"
#include "game/item_def.h"
#include "game/world.h"

#include "graphics/tileset.h"

#include "render/graphics.h"
#include "render/palette.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/ribbon.h"
#include "client/text.h"
#include "client/view.h"

#include "ui/list.h"
#include "ui/rectcut.h"
#include "ui/str_wrap.h"

#define MAX_DETAIL_LINES 8
// columns the item list takes beyond half the viewport
#define LIST_EXTRA_COLUMNS 2

/** Held items of the same type and level, shown as one row. */
struct item_group
{
  /** The first held item of this type and level. */
  handle(rl_item) first;
  /** How many items of this type and level are held. */
  int count;
};

alist_define_as(struct item_group, item_group);

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
  // the rows, in the order each item type and level was first found
  alist(item_group) groups;
  // the width of the item list, in glyphs
  int list_columns;
  // where the selected item's description goes
  SDL_FRect detail;
  float line_height;
  int detail_columns;
  int detail_capacity;
  // the selected item's description, wrapped
  struct rl_text detail_lines[MAX_DETAIL_LINES];
  int detail_count;
};

static bool
same_group(struct rl_item const* a, struct rl_item const* b)
{
  return a->itype == b->itype && a->level == b->level;
}

static void
build_groups(struct view_state* s)
{
  handle(rl_actor) const rogue = rl_get_rogue(s->world);
  int const count = rl_count_held_items(s->world, rogue);

  alist_clear(&s->groups);
  for (int i = 0; i < count; ++i) {
    handle(rl_item) const item_handle = rl_find_held_item(s->world, rogue, i);
    struct rl_item const* item = rl_borrow_item(s->world, item_handle);

    size_t g = 0;
    while (g < alist_len(&s->groups) &&
           !same_group(rl_borrow_item(s->world, alist_at(&s->groups, g)->first),
                       item)) {
      ++g;
    }

    if (g == alist_len(&s->groups)) {
      *alist_push(&s->groups) =
        (struct item_group){ .first = item_handle, .count = 0 };
    }
    ++alist_at(&s->groups, g)->count;
  }
}

static struct ui_list_menu_model
menu_model(struct view_state const* s)
{
  return (struct ui_list_menu_model){
    .count = (int)alist_len(&s->groups),
  };
}

static handle(rl_item)
selected_item(struct view_state const* s)
{
  if (s->menu.selected < 0 || s->menu.selected >= (int)alist_len(&s->groups)) {
    return handle_invalid(rl_item);
  }
  return alist_at(&s->groups, s->menu.selected)->first;
}

static void
init_view_state(struct view_state* s,
                struct rl_world const* world,
                SDL_FRect const* viewport,
                struct gfx_tileset const* font)
{
  float const glyph_width = (float)font->tile_width;
  float const line_height = (float)font->tile_height;

  s->world = world;
  s->menu.selected = -1;
  s->pending_item = handle_invalid(rl_item);

  SDL_FRect remaining = *viewport;
  float const half = SDL_floorf(remaining.w / 2.0f / glyph_width) * glyph_width;
  float const list_width =
    SDL_min(remaining.w, half + LIST_EXTRA_COLUMNS * glyph_width);
  SDL_FRect const left = ui_cut_left(&remaining, list_width);
  ui_cut_left(&remaining, SDL_min(remaining.w, glyph_width));

  s->list_columns = (int)(list_width / glyph_width);
  s->detail = remaining;
  s->line_height = line_height;
  s->detail_columns = (int)(remaining.w / glyph_width);
  s->detail_capacity =
    SDL_min(MAX_DETAIL_LINES, (int)(remaining.h / line_height));

  ui_list_init(
    &s->menu.list, &left, s->slots, SDL_arraysize(s->slots), line_height, 2.0f);
  build_groups(s);
  ui_list_menu_sync(&s->menu, menu_model(s));
}

static struct rl_text
item_text(struct rl_item const* item, int count, bool selected)
{
  SDL_FColor const colour = rl_get_item_gfx(item).fg;
  struct rl_text text = { 0 };

  char name[RL_TEXT_CAPACITY];
  rl_format_item_name(item, name, sizeof(name));

  rl_append_text(&text, &RL_COLOUR_YELLOW[3], selected ? "> " : "  ");
  rl_append_text(&text, &colour, name);
  if (count > 1) {
    char suffix[16];
    SDL_snprintf(suffix, sizeof(suffix), " (x%d)", count);
    rl_append_text(&text, &RL_COLOUR_GRAY[5], suffix);
  }

  return text;
}

static void
format_consumable(struct rl_item const* item,
                  struct rl_item_consumable_def const* idef,
                  char* buf,
                  size_t size)
{
  struct rl_roll_range const range = rl_get_roll_range(rl_get_item_power(item));

  switch (idef->effect) {
    case RL_ITEM_EFFECT_HEAL:
      SDL_snprintf(buf, size, "Heals %d to %d HP.", range.min, range.max);
      break;
    case RL_ITEM_EFFECT_DAMAGE_AREA:
      SDL_snprintf(buf,
                   size,
                   "Deals %d to %d damage to everything within %d tiles of "
                   "a chosen spot.",
                   range.min,
                   range.max,
                   idef->area_radius);
      break;
    case RL_ITEM_EFFECT_DAMAGE_NEAREST:
      SDL_snprintf(buf,
                   size,
                   "Deals %d to %d damage to the nearest visible enemy.",
                   range.min,
                   range.max);
      break;
  }
}

static void
format_description(struct rl_item const* item, char* buf, size_t size)
{
  buf[0] = '\0';

  struct rl_item_consumable_def const* idef =
    rl_get_item_consumable_def(item->itype);
  if (idef != NULL) {
    format_consumable(item, idef, buf, size);
  }
}

static void
refresh_detail(struct view_state* s)
{
  s->detail_count = 0;

  struct rl_item const* item = rl_borrow_item(s->world, selected_item(s));
  if (item == NULL) {
    return;
  }

  char description[RL_TEXT_CAPACITY];
  format_description(item, description, sizeof(description));

  char const* cursor = description;
  struct str_view line;
  while (s->detail_count < s->detail_capacity &&
         ui_wrap_next(&cursor, s->detail_columns, &line)) {
    struct rl_text* text = &s->detail_lines[s->detail_count++];
    *text = (struct rl_text){ 0 };
    rl_append_text_format(text, NULL, "%.*s", line.length, line.data);
  }
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
    rl_append_text(hint, colour, "[E] use");
  }
  if (hint->length > 0) {
    rl_append_text(hint, colour, "   ");
  }
  rl_append_text(hint, colour, "[Q] back");
}

static bool
handle_input(void* data, struct inpt_state const* istate)
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
prepare_view(void* data, float dt)
{
  (void)dt;
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  // Keep rows and selection in bounds after consuming an item.
  build_groups(s);
  ui_list_menu_sync(&s->menu, menu_model(s));
  refresh_detail(s);
}

static void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct gfx_tileset const* font)
{
  struct view_state const* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  int const count = (int)alist_len(&s->groups);
  int const first = ui_list_offset(&s->menu.list, count);

  for (int row = 0; row < s->menu.list.slot_count && first + row < count;
       ++row) {
    struct item_group const* group = alist_at(&s->groups, first + row);
    struct rl_item const* item = rl_borrow_item(s->world, group->first);

    struct rl_text text =
      item_text(item, group->count, first + row == s->menu.selected);
    rl_truncate_text(&text, (size_t)s->list_columns);

    SDL_FPoint const at = {
      s->slots[row].x,
      s->slots[row].y,
    };
    rl_draw_text(renderer, font, &text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);
  }

  for (int i = 0; i < s->detail_count; ++i) {
    SDL_FPoint const at = {
      s->detail.x,
      s->detail.y + (float)i * s->line_height,
    };
    rl_draw_text(renderer,
                 font,
                 &s->detail_lines[i],
                 RL_COLOUR_GRAY[5],
                 RL_COLOUR_BLACK,
                 at);
  }
}

static void
free_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  alist_free(&s->groups);
  SDL_free(s);
}

bool
rl_alloc_inv_view(struct rl_view* view,
                  struct rl_world const* world,
                  SDL_FRect const* viewport,
                  struct gfx_tileset const* font)
{
  struct view_state* s = SDL_calloc(1, sizeof(struct view_state));
  if (s == NULL) {
    return false;
  }

  if (!alist_alloc(&s->groups, RL_ITEM_TYPE_COUNT)) {
    SDL_free(s);
    return false;
  }

  view->state = s;
  init_view_state(s, world, viewport, font);

  view->free = free_view;
  view->describe_ribbon = describe_ribbon;
  view->handle_input = handle_input;
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
