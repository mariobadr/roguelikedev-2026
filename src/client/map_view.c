#include "map_view.h"

#include <SDL3/SDL_render.h>

#include "game/fov.h"
#include "game/tile.h"
#include "game/world.h"

#include "client/cell.h"
#include "client/font.h"
#include "client/graphics.h"
#include "client/lighting.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/ui.h"

static void
draw_level(SDL_Renderer* renderer,
           struct rl_font const* font,
           SDL_FRect const* panel,
           struct rl_level const* level,
           struct rl_fov const* fov)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  grid(rl_tile) const* map = &level->map;

  for (int y = 0; y < grid_height(map); y++) {
    for (int x = 0; x < grid_width(map); x++) {
      size_t const index = grid_index_of(map, x, y);

      if (!level->explored.data[index]) {
        // don't draw anything for unexplored tiles
        continue;
      }

      enum rl_tile const tile = *grid_at(map, x, y);
      struct rl_gfx_tile gfx = rl_get_tile_gfx(tile);

      if (!fov->visible.data[index]) {
        // dim explored but not visible tiles
        gfx.fg = rl_lerp_colour(gfx.fg, RL_COLOUR_BLACK, 0.4f);
      }

      SDL_FPoint const at = rl_panel_to_pixels(panel, (SDL_Point){ x, y });
      rl_draw_tile(renderer, font, &gfx, at);
    }
  }
}

static void
draw_light(SDL_Renderer* renderer,
           SDL_FRect const* panel,
           grid(rl_tile) const* map,
           struct rl_fov const* fov)
{
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
      SDL_FPoint const at = rl_panel_to_pixels(panel, (SDL_Point){ x, y });
      SDL_FRect const area = {
        at.x, at.y, (float)rl_cell_width(), (float)rl_cell_height()
      };

      SDL_SetRenderDrawColorFloat(
        renderer, colour.r, colour.g, colour.b, colour.a);
      SDL_RenderFillRect(renderer, &area);
    }
  }
}

static void
draw_item(SDL_Renderer* renderer,
          struct rl_font const* font,
          SDL_FRect const* panel,
          struct rl_item const* item)
{
  struct rl_gfx_tile const tile = rl_get_item_gfx(item);
  SDL_FPoint const at = rl_panel_to_pixels(panel, item->on.map);
  rl_draw_tile(renderer, font, &tile, at);
}

static void
draw_items(SDL_Renderer* renderer,
           struct rl_font const* font,
           SDL_FRect const* panel,
           struct rl_world const* world,
           struct rl_fov const* fov)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  for (int id = 0; id < alist_len(&world->items); id++) {
    struct rl_item const* item = rl_get_item(world, id);

    if (item->ltype != RL_ITEM_LOCATION_MAP) {
      continue;
    }

    if (*grid_at(&fov->visible, item->on.map.x, item->on.map.y)) {
      draw_item(renderer, font, panel, item);
    }
  }
}

static void
draw_actor(SDL_Renderer* renderer,
           struct rl_font const* font,
           SDL_FRect const* panel,
           struct rl_actor const* actor)
{
  struct rl_gfx_tile const tile = rl_get_actor_gfx(actor);
  SDL_FPoint const at = rl_panel_to_pixels(panel, actor->pos);
  rl_draw_tile(renderer, font, &tile, at);
}

static void
draw_actors(SDL_Renderer* renderer,
            struct rl_font const* font,
            SDL_FRect const* panel,
            struct rl_world const* world,
            struct rl_fov const* fov)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  for (int id = 0; id < rl_actor_count(world); id++) {
    struct rl_actor const* actor = rl_get_actor(world, id);

    if (!rl_actor_is_alive(actor)) {
      continue;
    }

    if (*grid_at(&fov->visible, actor->pos.x, actor->pos.y)) {
      draw_actor(renderer, font, panel, actor);
    }
  }
}

void
rl_draw_map(SDL_Renderer* renderer,
            struct rl_font const* font,
            SDL_FRect const* panel,
            struct rl_world const* world,
            struct rl_fov const* fov)
{
  struct rl_level const* level = rl_get_current_level(world);

  SDL_Rect const clip = rl_panel_clip_rect(panel);
  SDL_SetRenderClipRect(renderer, &clip);

  draw_level(renderer, font, panel, level, fov);
  draw_items(renderer, font, panel, world, fov);
  draw_actors(renderer, font, panel, world, fov);
  draw_light(renderer, panel, &level->map, fov);

  SDL_SetRenderClipRect(renderer, NULL);
}
