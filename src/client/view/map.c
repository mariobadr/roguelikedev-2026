#include "map.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "input/input.h"

#include "game/fov.h"
#include "game/targeting.h"
#include "game/tile.h"
#include "game/world.h"

#include "client/action.h"
#include "client/camera.h"
#include "client/controls.h"
#include "graphics/tileset.h"

#include "client/graphics.h"
#include "client/lighting.h"
#include "client/palette.h"
#include "client/ribbon.h"
#include "client/view.h"

/**
 * Possible input modes for the map.
 */
enum map_mode
{
  MAP_MODE_MOVE,   //< Move the rogue around
  MAP_MODE_SELECT, //< Move the cursor around
};

struct map_selection
{
  SDL_Point cursor;
  struct rl_item_def const* def;
  grid(boolean) mask;
  size_t capacity;
  SDL_Rect bounds;
};

struct view_state
{
  // the "model" for this view
  struct rl_world const* world;
  struct rl_fov const* fov;

  /** Which mode the view is in. */
  enum map_mode mode;

  // rendering state
  struct rl_camera camera;
  grid(gfx_console) terrain;
  grid(gfx_console) light;

  // for the MOVE mode
  struct rl_command pending_command;

  // for the SELECT mode
  struct map_selection selection;
  enum rl_map_selection_result pending_select;
  SDL_Point pending_point;
};

static bool
init_view_state(struct view_state* s,
                struct rl_world const* world,
                struct rl_fov const* fov,
                SDL_FRect const* viewport,
                int cell_width,
                int cell_height)
{
  s->world = world;
  s->fov = fov;

  rl_init_camera(&s->camera, viewport, cell_width, cell_height);

  if (!grid_alloc(&s->terrain, s->camera.bounds.w, s->camera.bounds.h)) {
    return false;
  }
  if (!grid_alloc(&s->light, s->camera.bounds.w, s->camera.bounds.h)) {
    grid_free(&s->terrain);
    return false;
  }

  return true;
}

static void
populate_terrain(struct view_state* s)
{
  struct rl_level const* level = rl_get_current_level(s->world);
  grid(rl_tile) const* map = &level->map;

  gfx_clear_console_grid(&s->terrain);

  SDL_Rect const visible =
    rl_visible_world(&s->camera, grid_width(map), grid_height(map));

  for (int y = visible.y; y < visible.y + visible.h; y++) {
    for (int x = visible.x; x < visible.x + visible.w; x++) {
      SDL_Point const p = { x, y };
      struct gfx_console_cell cell = { 0 };

      if (rl_is_tile_explored(level, p)) {
        enum rl_tile const tile = *grid_at(map, p.x, p.y);
        cell = rl_get_tile_gfx(tile);

        if (!rl_is_tile_visible(s->fov, p)) {
          // dim explored but not visible tiles
          cell.fg = rl_lerp_colour(cell.fg, RL_COLOUR_BLACK, 0.4f);
        }
      }

      SDL_Point const local = rl_world_to_grid(&s->camera, p);
      *grid_at(&s->terrain, local.x, local.y) = cell;
    }
  }
}

static void
populate_light(struct view_state* s)
{
  struct rl_level const* level = rl_get_current_level(s->world);
  grid(rl_tile) const* map = &level->map;

  gfx_clear_console_grid(&s->light);

  // the colour of the light source - make this an argument?
  SDL_FColor const light = RL_COLOUR_GRAY[6];

  SDL_Rect const visible =
    rl_visible_world(&s->camera, grid_width(map), grid_height(map));

  for (int y = visible.y; y < visible.y + visible.h; y++) {
    for (int x = visible.x; x < visible.x + visible.w; x++) {
      SDL_Point const p = { x, y };
      struct gfx_console_cell cell = { 0 };

      if (rl_is_tile_visible(s->fov, p)) {
        float const brightness =
          rl_calculate_brightness(s->fov->origin, p, (float)s->fov->radius);
        float const alpha = rl_lerp_float(0.6f, 0.0f, brightness);

        cell.bg = (SDL_FColor){ light.r, light.g, light.b, alpha };
      }

      SDL_Point const local = rl_world_to_grid(&s->camera, p);
      *grid_at(&s->light, local.x, local.y) = cell;
    }
  }
}

static void
draw_level(struct view_state const* s,
           SDL_Renderer* renderer,
           struct gfx_tileset const* font)
{
  gfx_draw_console_grid(
    renderer, font, &s->terrain, NULL, rl_viewport_origin(&s->camera));
}

static void
draw_light(struct view_state const* s,
           SDL_Renderer* renderer,
           struct gfx_tileset const* font)
{
  SDL_BlendMode prev;
  SDL_GetRenderDrawBlendMode(renderer, &prev);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

  gfx_draw_console_grid(
    renderer, font, &s->light, NULL, rl_viewport_origin(&s->camera));

  SDL_SetRenderDrawBlendMode(renderer, prev);
}

static void
draw_item(struct view_state const* s,
          SDL_Renderer* renderer,
          struct gfx_tileset const* font,
          struct rl_item const* item)
{
  struct gfx_console_cell const cell = rl_get_item_gfx(item);
  SDL_FPoint const at = rl_world_to_screen(&s->camera, item->on.map);
  SDL_FRect dst = gfx_tileset_dst(font, at, 1);
  gfx_draw_console_cell(renderer, font, &cell, &dst);
}

static void
draw_items(struct view_state const* s,
           SDL_Renderer* renderer,
           struct gfx_tileset const* font)
{
  struct rl_level const* level = rl_get_current_level(s->world);
  for (size_t i = 0; i < alist_len(&level->items); i++) {
    struct rl_item const* item =
      rl_borrow_item(s->world, *alist_at(&level->items, i));
    if (item == NULL) {
      continue;
    }

    if (rl_is_tile_visible(s->fov, item->on.map)) {
      draw_item(s, renderer, font, item);
    }
  }
}

static void
draw_actor(struct view_state const* s,
           SDL_Renderer* renderer,
           struct gfx_tileset const* font,
           struct rl_actor const* actor)
{
  struct gfx_console_cell const cell = rl_get_actor_gfx(actor);
  SDL_FPoint const at = rl_world_to_screen(&s->camera, actor->pos);
  SDL_FRect dst = gfx_tileset_dst(font, at, 1);
  gfx_draw_console_cell(renderer, font, &cell, &dst);
}

static void
draw_actors(struct view_state const* s,
            SDL_Renderer* renderer,
            struct gfx_tileset const* font)
{
  struct rl_level const* level = rl_get_current_level(s->world);
  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    struct rl_actor const* actor =
      rl_borrow_actor(s->world, *alist_at(&level->actors, i));

    if (actor == NULL || !rl_actor_is_alive(actor)) {
      continue;
    }

    if (rl_is_tile_visible(s->fov, actor->pos)) {
      draw_actor(s, renderer, font, actor);
    }
  }
}

static bool
reserve_area(struct map_selection* selection, int w, int h)
{
  size_t const count = (size_t)w * (size_t)h;
  if (count > selection->capacity) {
    bool* data = SDL_realloc(selection->mask.data, count * sizeof(*data));
    if (data == NULL) {
      SDL_Log("SDL_realloc failed: %s", SDL_GetError());
      return false;
    }

    selection->mask.data = data;
    selection->capacity = count;
  }

  selection->mask.width = w;
  selection->mask.height = h;

  return true;
}

/**
 * Refresh after selecting an item or moving the cursor. The world cannot
 * change while selecting, and moving the cursor does not change mask
 * dimensions.
 */
static void
refresh_area(struct view_state* s)
{
  struct map_selection* selection = &s->selection;
  selection->bounds = rl_item_area_bounds(selection->def, selection->cursor);
  rl_fill_item_area(
    selection->def, s->world, selection->cursor, &selection->mask);
}

static void
draw_target_area(struct view_state const* s, SDL_Renderer* renderer)
{
  struct map_selection const* selection = &s->selection;

  grid(rl_tile) const* map = &rl_get_current_level(s->world)->map;
  SDL_Rect const visible =
    rl_visible_world(&s->camera, grid_width(map), grid_height(map));

  SDL_Rect region;
  if (!SDL_GetRectIntersection(&visible, &selection->bounds, &region)) {
    // nothing on camera
    return;
  }

  SDL_BlendMode prev;
  SDL_GetRenderDrawBlendMode(renderer, &prev);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_FColor const colour = RL_COLOUR_RED[5];
  SDL_SetRenderDrawColorFloat(renderer, colour.r, colour.g, colour.b, 0.35f);

  for (int y = region.y; y < region.y + region.h; y++) {
    for (int x = region.x; x < region.x + region.w; x++) {
      if (!*grid_at(&selection->mask,
                    x - selection->bounds.x,
                    y - selection->bounds.y)) {
        continue;
      }

      SDL_Point const p = { x, y };
      SDL_FPoint const at = rl_world_to_screen(&s->camera, p);
      SDL_FRect rect = { 0 };
      rect.x = at.x;
      rect.y = at.y;
      rect.w = (float)s->camera.cell_width;
      rect.h = (float)s->camera.cell_height;
      SDL_RenderFillRect(renderer, &rect);
    }
  }

  SDL_SetRenderDrawBlendMode(renderer, prev);
}

static void
draw_cursor(struct view_state const* s, SDL_Renderer* renderer)
{
  SDL_FPoint const at = rl_world_to_screen(&s->camera, s->selection.cursor);
  SDL_FRect rect = { 0 };
  rect.x = at.x;
  rect.y = at.y;
  rect.w = (float)s->camera.cell_width;
  rect.h = (float)s->camera.cell_height;

  SDL_FColor colour = RL_COLOUR_YELLOW[5];
  SDL_SetRenderDrawColorFloat(renderer, colour.r, colour.g, colour.b, colour.a);
  SDL_RenderRect(renderer, &rect);
}

static void
describe_ribbon(void const* data, struct rl_ribbon_content* content)
{
  struct view_state const* s = (struct view_state const*)data;
  SDL_assert(s != NULL);

  rl_append_text(&content->text[RL_RIBBON_LEFT], NULL, "View: Map");

  struct rl_text* mode = &content->text[RL_RIBBON_CENTRE];
  struct rl_text* hint = &content->text[RL_RIBBON_RIGHT];
  if (s->mode == MAP_MODE_SELECT) {
    rl_append_text(mode, NULL, "Selecting");
    rl_append_text(hint, &RL_COLOUR_YELLOW[3], "[WASD, E, Esc]");
  } else {
    rl_append_text(mode, NULL, "Moving");
    rl_append_text(hint, &RL_COLOUR_YELLOW[3], "[WASD, E, Z]");
  }
}

static bool
update_move(struct view_state* s, struct inpt_state const* istate)
{
  struct rl_actor const* rogue =
    rl_borrow_actor(s->world, rl_get_rogue(s->world));
  if (rogue == NULL) {
    return false;
  }

  enum rl_action action = rl_handle_keyboard_input(istate);
  if (action == RL_ACTION_NONE) {
    grid(rl_tile) const* map = &rl_get_current_level(s->world)->map;

    SDL_Point target;
    if (rl_get_world_cell(&s->camera, istate->mouse.position, &target) &&
        grid_contains(map, target.x, target.y)) {
      action = rl_handle_mouse_input(istate, rogue->pos, target);
    }
  }

  switch (action) {
    case RL_ACTION_MOVE_UP:
    case RL_ACTION_MOVE_DOWN:
    case RL_ACTION_MOVE_LEFT:
    case RL_ACTION_MOVE_RIGHT:
    case RL_ACTION_SELECT:
    case RL_ACTION_WAIT:
      s->pending_command = rl_build_command(rogue, action, s->world);
      return true;
    default:
      return false;
  }
}

static void
end_select(struct view_state* s, enum rl_map_selection_result result)
{
  s->pending_select = result;
  s->mode = MAP_MODE_MOVE;
  s->selection.def = NULL;
}

static bool
update_select(struct view_state* s, struct inpt_state const* istate)
{
  enum rl_action const action = rl_handle_keyboard_input(istate);

  SDL_Point next = s->selection.cursor;
  switch (action) {
    case RL_ACTION_MOVE_UP:
      next.y -= 1;
      break;
    case RL_ACTION_MOVE_DOWN:
      next.y += 1;
      break;
    case RL_ACTION_MOVE_LEFT:
      next.x -= 1;
      break;
    case RL_ACTION_MOVE_RIGHT:
      next.x += 1;
      break;
    case RL_ACTION_SELECT:
      s->pending_point = s->selection.cursor;
      end_select(s, RL_MAP_SELECTION_CONFIRMED);
      return true;
    case RL_ACTION_CANCEL:
      end_select(s, RL_MAP_SELECTION_CANCELLED);
      return true;
    default:
      return false;
  }

  if (rl_is_valid_item_target(s->selection.def, s->world, s->fov, next)) {
    s->selection.cursor = next;
    refresh_area(s);
  }

  return true;
}

static bool
update_view(void* data, struct inpt_state const* istate)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  if (s->mode == MAP_MODE_SELECT) {
    return update_select(s, istate);
  }

  return update_move(s, istate);
}

static void
prepare_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  struct rl_actor const* rogue =
    rl_borrow_actor(s->world, rl_get_rogue(s->world));
  grid(rl_tile) const* map = &rl_get_current_level(s->world)->map;

  SDL_Point const origin =
    s->mode == MAP_MODE_SELECT ? s->selection.cursor : rogue->pos;
  rl_centre_camera_on(&s->camera, origin, grid_width(map), grid_height(map));

  populate_terrain(s);
  populate_light(s);
}

static void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct gfx_tileset const* font)
{
  struct view_state const* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  draw_level(s, renderer, font);
  draw_items(s, renderer, font);
  draw_actors(s, renderer, font);
  draw_light(s, renderer, font);

  if (s->mode == MAP_MODE_SELECT) {
    draw_target_area(s, renderer);
    draw_cursor(s, renderer);
  }
}

static void
free_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  if (s == NULL) {
    return;
  }

  grid_free(&s->selection.mask);
  grid_free(&s->terrain);
  grid_free(&s->light);
  SDL_free(s);
}

bool
rl_alloc_map_view(struct rl_view* view,
                  struct rl_world const* world,
                  struct rl_fov const* fov,
                  SDL_FRect const* viewport,
                  int cell_width,
                  int cell_height)
{
  SDL_assert(SDL_fmodf(viewport->w, (float)cell_width) == 0.0f);
  SDL_assert(SDL_fmodf(viewport->h, (float)cell_height) == 0.0f);

  view->state = SDL_calloc(1, sizeof(struct view_state));
  if (view->state == NULL) {
    return false;
  }

  if (!init_view_state(
        view->state, world, fov, viewport, cell_width, cell_height)) {
    free_view(view->state);
    view->state = NULL;
    return false;
  }

  view->free = free_view;
  view->describe_ribbon = describe_ribbon;
  view->update = update_view;
  view->prepare = prepare_view;
  view->render = render_view;

  return true;
}

bool
rl_map_view_take_command(struct rl_view* view, struct rl_command* out)
{
  struct view_state* s = view->state;
  SDL_assert(s != NULL);

  if (s->pending_command.type == RL_COMMAND_NONE) {
    return false;
  }

  *out = s->pending_command;
  s->pending_command = (struct rl_command){ 0 };

  return true;
}

bool
rl_map_view_begin_select(struct rl_view* view,
                         SDL_Point origin,
                         struct rl_item_def const* def)
{
  SDL_assert(def != NULL);

  struct view_state* s = (struct view_state*)view->state;
  SDL_assert(s != NULL);

  // Reserve storage before entering selection mode.
  SDL_Rect const bounds = rl_item_area_bounds(def, origin);
  if (!reserve_area(&s->selection, bounds.w, bounds.h)) {
    return false;
  }

  s->selection.cursor = origin;
  s->selection.def = def;
  refresh_area(s);
  s->pending_select = RL_MAP_SELECTION_NONE;
  s->mode = MAP_MODE_SELECT;

  return true;
}

enum rl_map_selection_result
rl_map_view_take_selection(struct rl_view* view, SDL_Point* out)
{
  SDL_assert(out != NULL);

  struct view_state* s = (struct view_state*)view->state;
  SDL_assert(s != NULL);

  enum rl_map_selection_result const result = s->pending_select;
  if (result != RL_MAP_SELECTION_NONE) {
    if (result == RL_MAP_SELECTION_CONFIRMED) {
      *out = s->pending_point;
    }

    // reset before returning
    s->pending_select = RL_MAP_SELECTION_NONE;
  }

  return result;
}
