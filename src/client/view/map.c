#include "map.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "input/input.h"

#include "game/fov.h"
#include "game/tile.h"
#include "game/world.h"

#include "client/view/ribbon.h"

#include "client/action.h"
#include "client/camera.h"
#include "client/controls.h"
#include "client/graphics.h"
#include "client/lighting.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/view.h"

struct view_state
{
  struct rl_world const* world;
  struct rl_fov const* fov;
  /** Where this view exists on the screen. */
  SDL_FRect viewport;
  /** The width of a cell in the map. */
  int cell_width;
  /** The height of a cell in the map. */
  int cell_height;
  /** A "camera" of what's currently visible. */
  SDL_Rect camera;
};

static void
init_view_state(struct view_state* s,
                struct rl_world const* world,
                struct rl_fov const* fov,
                SDL_FRect const* viewport,
                int cell_width,
                int cell_height)
{
  s->world = world;
  s->fov = fov;
  s->viewport = *viewport;
  s->cell_width = cell_width;
  s->cell_height = cell_height;

  s->camera.x = 0;
  s->camera.y = 0;
  s->camera.w = (int)viewport->w / cell_width;
  s->camera.h = (int)viewport->h / cell_height;
}

static SDL_FPoint
cell_origin(SDL_FRect const* viewport)
{
  SDL_FPoint origin = { 0 };
  origin.x = SDL_floorf(viewport->x);
  origin.y = SDL_floorf(viewport->y);

  return origin;
}

static SDL_FPoint
cell_to_pixels(struct view_state const* s, SDL_Point cell)
{
  SDL_FPoint const origin = cell_origin(&s->viewport);

  SDL_FPoint pixels = { 0 };
  pixels.x = origin.x + (float)((cell.x - s->camera.x) * s->cell_width);
  pixels.y = origin.y + (float)((cell.y - s->camera.y) * s->cell_height);

  return pixels;
}

static bool
cell_at(struct view_state const* s, SDL_FPoint pos, SDL_Point* cell)
{
  if (!SDL_PointInRectFloat(&pos, &s->viewport)) {
    return false;
  }

  SDL_FPoint const origin = cell_origin(&s->viewport);

  SDL_Point world = { 0 };
  world.x =
    (int)SDL_floorf((pos.x - origin.x) / (float)s->cell_width) + s->camera.x;
  world.y =
    (int)SDL_floorf((pos.y - origin.y) / (float)s->cell_height) + s->camera.y;

  grid(rl_tile) const* map = &rl_get_current_level(s->world)->map;
  if (!grid_contains(map, world.x, world.y)) {
    return false;
  }

  *cell = world;
  return true;
}

static void
draw_level(struct view_state const* s,
           SDL_Renderer* renderer,
           struct rl_font const* font)
{
  struct rl_level const* level = rl_get_current_level(s->world);
  grid(rl_tile) const* map = &level->map;

  SDL_Rect const visible =
    rl_visible_world(&s->camera, grid_width(map), grid_height(map));

  for (int y = visible.y; y < visible.y + visible.h; y++) {
    for (int x = visible.x; x < visible.x + visible.w; x++) {
      size_t const index = grid_index_of(map, x, y);

      if (!level->explored.data[index]) {
        // don't draw anything for unexplored tiles
        continue;
      }

      enum rl_tile const tile = *grid_at(map, x, y);
      struct rl_cell cell = rl_get_tile_gfx(tile);

      if (!s->fov->visible.data[index]) {
        // dim explored but not visible tiles
        cell.fg = rl_lerp_colour(cell.fg, RL_COLOUR_BLACK, 0.4f);
      }

      SDL_FPoint const at = cell_to_pixels(s, (SDL_Point){ x, y });
      rl_draw_cell(renderer, font, &cell, at);
    }
  }
}

static void
draw_light(struct view_state const* s, SDL_Renderer* renderer)
{
  SDL_BlendMode prev;
  SDL_GetRenderDrawBlendMode(renderer, &prev);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

  struct rl_level const* level = rl_get_current_level(s->world);
  grid(rl_tile) const* map = &level->map;

  // the colour of the light source - make this an argument?
  SDL_FColor const light = RL_COLOUR_GRAY[6];

  SDL_Rect const visible =
    rl_visible_world(&s->camera, grid_width(map), grid_height(map));

  for (int y = visible.y; y < visible.y + visible.h; y++) {
    for (int x = visible.x; x < visible.x + visible.w; x++) {
      size_t const index = grid_index_of(map, x, y);

      if (!s->fov->visible.data[index]) {
        // not visible, so there's no "glow" to add
        continue;
      }

      float const brightness = rl_calculate_brightness(
        s->fov->origin, (SDL_Point){ x, y }, (float)s->fov->radius);
      float const alpha = rl_lerp_float(0.6f, 0.0f, brightness);

      SDL_FColor const colour = { light.r, light.g, light.b, alpha };
      SDL_FPoint const at = cell_to_pixels(s, (SDL_Point){ x, y });

      SDL_FRect dst = { 0 };
      dst.x = at.x;
      dst.y = at.y;
      dst.w = (float)s->cell_width;
      dst.h = (float)s->cell_height;

      SDL_SetRenderDrawColorFloat(
        renderer, colour.r, colour.g, colour.b, colour.a);
      SDL_RenderFillRect(renderer, &dst);
    }
  }

  SDL_SetRenderDrawBlendMode(renderer, prev);
}

static void
draw_item(struct view_state const* s,
          SDL_Renderer* renderer,
          struct rl_font const* font,
          struct rl_item const* item)
{
  struct rl_cell const cell = rl_get_item_gfx(item);
  SDL_FPoint const at = cell_to_pixels(s, item->on.map);
  rl_draw_cell(renderer, font, &cell, at);
}

static void
draw_items(struct view_state const* s,
           SDL_Renderer* renderer,
           struct rl_font const* font)
{
  for (int id = 0; id < alist_len(&s->world->items); id++) {
    struct rl_item const* item = rl_get_item(s->world, id);

    if (item->ltype != RL_ITEM_LOCATION_MAP) {
      continue;
    }

    if (*grid_at(&s->fov->visible, item->on.map.x, item->on.map.y)) {
      draw_item(s, renderer, font, item);
    }
  }
}

static void
draw_actor(struct view_state const* s,
           SDL_Renderer* renderer,
           struct rl_font const* font,
           struct rl_actor const* actor)
{
  struct rl_cell const cell = rl_get_actor_gfx(actor);
  SDL_FPoint const at = cell_to_pixels(s, actor->pos);
  rl_draw_cell(renderer, font, &cell, at);
}

static void
draw_actors(struct view_state const* s,
            SDL_Renderer* renderer,
            struct rl_font const* font)
{
  for (int id = 0; id < rl_actor_count(s->world); id++) {
    struct rl_actor const* actor = rl_get_actor(s->world, id);

    if (!rl_actor_is_alive(actor)) {
      continue;
    }

    if (*grid_at(&s->fov->visible, actor->pos.x, actor->pos.y)) {
      draw_actor(s, renderer, font, actor);
    }
  }
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

  rl_set_current_view(ribbon, "Map");
  rl_set_current_mode(ribbon, "Moving");

  struct rl_text msg = { 0 };
  rl_append_text(&msg, &RL_COLOUR_YELLOW[3], "[WASD, E]");
  rl_set_ribbon_text(ribbon, RL_RIBBON_RIGHT, &msg);
}

static bool
update_view(void* data, struct inpt_state const* istate, struct rl_command* out)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  enum rl_action action = rl_handle_keyboard_input(istate);
  if (action == RL_ACTION_NONE) {
    SDL_Point target;
    if (cell_at(s, istate->mouse.position, &target)) {
      struct rl_actor const* rogue = rl_get_actor(s->world, RL_ROGUE_ID);
      action = rl_handle_mouse_input(istate, rogue->pos, target);
    }
  }

  switch (action) {
    case RL_ACTION_MOVE_UP:
    case RL_ACTION_MOVE_DOWN:
    case RL_ACTION_MOVE_LEFT:
    case RL_ACTION_MOVE_RIGHT:
    case RL_ACTION_SELECT:
      *out = rl_build_command(RL_ROGUE_ID, action, s->world);
      return true;
    default:
      return false;
  }
}

static void
prepare_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  struct rl_actor const* rogue = rl_get_actor(s->world, RL_ROGUE_ID);
  grid(rl_tile) const* map = &rl_get_current_level(s->world)->map;

  rl_centre_camera_on(
    &s->camera, rogue->pos, grid_width(map), grid_height(map));
}

void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct rl_font const* font)
{
  struct view_state const* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  draw_level(s, renderer, font);
  draw_items(s, renderer, font);
  draw_actors(s, renderer, font);
  draw_light(s, renderer);
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

  init_view_state(view->state, world, fov, viewport, cell_width, cell_height);

  view->can_interact = can_interact;
  view->free = SDL_free;
  view->update_ribbon = update_ribbon;
  view->update = update_view;
  view->prepare = prepare_view;
  view->render = render_view;

  return true;
}
