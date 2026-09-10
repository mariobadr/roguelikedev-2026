#include "map.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "game/fov.h"
#include "game/tile.h"
#include "game/world.h"

#include "client/graphics.h"
#include "client/lighting.h"
#include "client/palette.h"
#include "client/render.h"

static SDL_FPoint
cell_origin(SDL_FRect const* viewport)
{
  SDL_FPoint origin = { 0 };
  origin.x = SDL_floorf(viewport->x);
  origin.y = SDL_floorf(viewport->y);

  return origin;
}

static SDL_FPoint
cell_to_pixels(struct rl_map_view const* map, SDL_Point cell)
{
  SDL_FPoint const origin = cell_origin(&map->viewport);

  SDL_FPoint pixels = { 0 };
  pixels.x = origin.x + (float)(cell.x * map->cell_width);
  pixels.y = origin.y + (float)(cell.y * map->cell_height);

  return pixels;
}

static void
draw_level(struct rl_map_view const* map_view,
           SDL_Renderer* renderer,
           struct rl_font const* font,
           struct rl_level const* level,
           struct rl_fov const* fov)
{
  grid(rl_tile) const* map = &level->map;

  for (int y = 0; y < grid_height(map); y++) {
    for (int x = 0; x < grid_width(map); x++) {
      size_t const index = grid_index_of(map, x, y);

      if (!level->explored.data[index]) {
        // don't draw anything for unexplored tiles
        continue;
      }

      enum rl_tile const tile = *grid_at(map, x, y);
      struct rl_cell cell = rl_get_tile_gfx(tile);

      if (!fov->visible.data[index]) {
        // dim explored but not visible tiles
        cell.fg = rl_lerp_colour(cell.fg, RL_COLOUR_BLACK, 0.4f);
      }

      SDL_FPoint const at = cell_to_pixels(map_view, (SDL_Point){ x, y });
      rl_draw_cell(renderer, font, &cell, at);
    }
  }
}

static void
draw_light(struct rl_map_view const* map_view,
           SDL_Renderer* renderer,
           grid(rl_tile) const* map,
           struct rl_fov const* fov)
{
  SDL_BlendMode prev;
  SDL_GetRenderDrawBlendMode(renderer, &prev);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

  // the colour of the light source - make this an argument?
  SDL_FColor const light = RL_COLOUR_GRAY[6];

  for (int y = 0; y < grid_height(map); y++) {
    for (int x = 0; x < grid_width(map); x++) {
      size_t const index = grid_index_of(map, x, y);

      if (!fov->visible.data[index]) {
        // not visible, so there's no "glow" to add
        continue;
      }

      float const brightness = rl_calculate_brightness(
        fov->origin, (SDL_Point){ x, y }, (float)fov->radius);
      float const alpha = rl_lerp_float(0.6f, 0.0f, brightness);

      SDL_FColor const colour = { light.r, light.g, light.b, alpha };
      SDL_FPoint const at = cell_to_pixels(map_view, (SDL_Point){ x, y });

      SDL_FRect dst = { 0 };
      dst.x = at.x;
      dst.y = at.y;
      dst.w = (float)map_view->cell_width;
      dst.h = (float)map_view->cell_height;

      SDL_SetRenderDrawColorFloat(
        renderer, colour.r, colour.g, colour.b, colour.a);
      SDL_RenderFillRect(renderer, &dst);
    }
  }

  SDL_SetRenderDrawBlendMode(renderer, prev);
}

static void
draw_item(struct rl_map_view const* map_view,
          SDL_Renderer* renderer,
          struct rl_font const* font,
          struct rl_item const* item)
{
  struct rl_cell const cell = rl_get_item_gfx(item);
  SDL_FPoint const at = cell_to_pixels(map_view, item->on.map);
  rl_draw_cell(renderer, font, &cell, at);
}

static void
draw_items(struct rl_map_view const* map_view,
           SDL_Renderer* renderer,
           struct rl_font const* font,
           struct rl_world const* world,
           struct rl_fov const* fov)
{
  for (int id = 0; id < alist_len(&world->items); id++) {
    struct rl_item const* item = rl_get_item(world, id);

    if (item->ltype != RL_ITEM_LOCATION_MAP) {
      continue;
    }

    if (*grid_at(&fov->visible, item->on.map.x, item->on.map.y)) {
      draw_item(map_view, renderer, font, item);
    }
  }
}

static void
draw_actor(struct rl_map_view const* map_view,
           SDL_Renderer* renderer,
           struct rl_font const* font,
           struct rl_actor const* actor)
{
  struct rl_cell const cell = rl_get_actor_gfx(actor);
  SDL_FPoint const at = cell_to_pixels(map_view, actor->pos);
  rl_draw_cell(renderer, font, &cell, at);
}

static void
draw_actors(struct rl_map_view const* map_view,
            SDL_Renderer* renderer,
            struct rl_font const* font,
            struct rl_world const* world,
            struct rl_fov const* fov)
{
  for (int id = 0; id < rl_actor_count(world); id++) {
    struct rl_actor const* actor = rl_get_actor(world, id);

    if (!rl_actor_is_alive(actor)) {
      continue;
    }

    if (*grid_at(&fov->visible, actor->pos.x, actor->pos.y)) {
      draw_actor(map_view, renderer, font, actor);
    }
  }
}

void
rl_init_map_view(struct rl_map_view* map,
                 SDL_FRect const* viewport,
                 int cell_width,
                 int cell_height)
{
  SDL_assert(SDL_fmodf(viewport->w, (float)cell_width) == 0.0f);
  SDL_assert(SDL_fmodf(viewport->h, (float)cell_height) == 0.0f);

  map->viewport = *viewport;
  map->cell_width = cell_width;
  map->cell_height = cell_height;
}

void
rl_map_view_size(struct rl_map_view const* map, int* width, int* height)
{
  if (width != NULL) {
    *width = (int)map->viewport.w / map->cell_width;
  }

  if (height != NULL) {
    *height = (int)map->viewport.h / map->cell_height;
  }
}

bool
rl_map_view_cell_at(struct rl_map_view const* map,
                    SDL_FPoint pos,
                    SDL_Point* cell)
{
  if (!SDL_PointInRectFloat(&pos, &map->viewport)) {
    return false;
  }

  SDL_FPoint const origin = cell_origin(&map->viewport);

  cell->x = (int)SDL_floorf((pos.x - origin.x) / (float)map->cell_width);
  cell->y = (int)SDL_floorf((pos.y - origin.y) / (float)map->cell_height);

  return true;
}

void
rl_draw_map_view(struct rl_map_view const* map_view,
                 SDL_Renderer* renderer,
                 struct rl_font const* font,
                 struct rl_world const* world,
                 struct rl_fov const* fov)
{
  struct rl_level const* level = rl_get_current_level(world);

  draw_level(map_view, renderer, font, level, fov);
  draw_items(map_view, renderer, font, world, fov);
  draw_actors(map_view, renderer, font, world, fov);
  draw_light(map_view, renderer, &level->map, fov);
}
