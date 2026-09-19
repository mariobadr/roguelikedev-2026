#include "world.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "container/alist.h"

#include "game/actor.h"
#include "game/fov.h"
#include "game/item.h"
#include "game/level.h"
#include "game/tile.h"
#include "game/world.h"

#include "graphics/tileset.h"

#include "camera.h"
#include "graphics.h"
#include "lighting.h"
#include "palette.h"

static void
populate_terrain(struct rl_world_renderer* wr,
                 struct rl_world const* world,
                 struct rl_camera const* camera)
{
  struct rl_level const* level = rl_get_current_level(world);
  grid(rl_tile) const* map = &level->map;

  gfx_clear_console_grid(&wr->terrain);

  for (int y = wr->visible.y; y < wr->visible.y + wr->visible.h; y++) {
    for (int x = wr->visible.x; x < wr->visible.x + wr->visible.w; x++) {
      SDL_Point const p = { x, y };
      struct gfx_console_cell cell = { 0 };

      if (rl_is_tile_explored(level, p)) {
        enum rl_tile const tile = *grid_at(map, p.x, p.y);
        cell = rl_get_tile_gfx(tile);

        if (!rl_is_tile_visible(&world->player.fov, p)) {
          // dim explored but not visible tiles
          cell.fg = rl_lerp_colour(cell.fg, RL_COLOUR_BLACK, 0.4f);
        }
      }

      SDL_Point const local = rl_world_to_grid(camera, p);
      *grid_at(&wr->terrain, local.x, local.y) = cell;
    }
  }
}

static void
populate_light(struct rl_world_renderer* wr,
               struct rl_world const* world,
               struct rl_camera const* camera)
{
  gfx_clear_console_grid(&wr->light);

  // the colour of the light source - make this an argument?
  SDL_FColor const light = RL_COLOUR_GRAY[6];

  for (int y = wr->visible.y; y < wr->visible.y + wr->visible.h; y++) {
    for (int x = wr->visible.x; x < wr->visible.x + wr->visible.w; x++) {
      SDL_Point const p = { x, y };
      struct gfx_console_cell cell = { 0 };

      if (rl_is_tile_visible(&world->player.fov, p)) {
        float const brightness = rl_calculate_brightness(
          world->player.fov.origin, p, (float)world->player.fov.radius);
        float const alpha = rl_lerp_float(0.6f, 0.0f, brightness);

        cell.bg = (SDL_FColor){ light.r, light.g, light.b, alpha };
      }

      SDL_Point const local = rl_world_to_grid(camera, p);
      *grid_at(&wr->light, local.x, local.y) = cell;
    }
  }
}

static void
draw_level(struct rl_world_renderer const* wr,
           SDL_Renderer* renderer,
           struct gfx_tileset const* font,
           SDL_FPoint screen_origin)
{
  gfx_draw_console_grid(renderer, font, &wr->terrain, NULL, screen_origin);
}

static void
draw_light(struct rl_world_renderer const* wr,
           SDL_Renderer* renderer,
           struct gfx_tileset const* font,
           SDL_FPoint screen_origin)
{
  SDL_BlendMode prev;
  SDL_GetRenderDrawBlendMode(renderer, &prev);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

  gfx_draw_console_grid(renderer, font, &wr->light, NULL, screen_origin);

  SDL_SetRenderDrawBlendMode(renderer, prev);
}

static void
draw_item(struct rl_camera const* camera,
          SDL_Renderer* renderer,
          struct gfx_tileset const* font,
          struct rl_item const* item)
{
  struct gfx_console_cell const cell = rl_get_item_gfx(item);
  SDL_FPoint const at = rl_world_to_screen(camera, item->on.map);
  SDL_FRect dst = gfx_tileset_dst(font, at, 1);
  gfx_draw_console_cell(renderer, font, &cell, &dst);
}

static void
draw_items(struct rl_camera const* camera,
           struct rl_world const* world,
           SDL_Renderer* renderer,
           struct gfx_tileset const* font)
{
  struct rl_level const* level = rl_get_current_level(world);
  for (size_t i = 0; i < alist_len(&level->items); i++) {
    struct rl_item const* item =
      rl_borrow_item(world, *alist_at(&level->items, i));
    if (item == NULL) {
      continue;
    }

    if (rl_is_tile_visible(&world->player.fov, item->on.map)) {
      draw_item(camera, renderer, font, item);
    }
  }
}

static void
draw_actor(struct rl_camera const* camera,
           SDL_Renderer* renderer,
           struct gfx_tileset const* font,
           struct rl_actor const* actor)
{
  struct gfx_console_cell const cell = rl_get_actor_gfx(actor);
  SDL_FPoint const at = rl_world_to_screen(camera, actor->pos);
  SDL_FRect dst = gfx_tileset_dst(font, at, 1);
  gfx_draw_console_cell(renderer, font, &cell, &dst);
}

static void
draw_actors(struct rl_camera const* camera,
            struct rl_world const* world,
            SDL_Renderer* renderer,
            struct gfx_tileset const* font)
{
  struct rl_level const* level = rl_get_current_level(world);
  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    struct rl_actor const* actor =
      rl_borrow_actor(world, *alist_at(&level->actors, i));

    if (actor == NULL || !rl_actor_is_alive(actor)) {
      continue;
    }

    if (rl_is_tile_visible(&world->player.fov, actor->pos)) {
      draw_actor(camera, renderer, font, actor);
    }
  }
}

bool
rl_init_world_renderer(struct rl_world_renderer* wr,
                     int columns,
                     int rows)
{
  wr->visible = (SDL_Rect){ 0 };

  if (!grid_alloc(&wr->terrain, columns, rows)) {
    return false;
  }
  if (!grid_alloc(&wr->light, columns, rows)) {
    grid_free(&wr->terrain);
    return false;
  }

  return true;
}

void
rl_free_world_renderer(struct rl_world_renderer* wr)
{
  grid_free(&wr->terrain);
  grid_free(&wr->light);
}

void
rl_prepare_world_renderer(struct rl_world_renderer* wr,
                        struct rl_world const* world,
                        struct rl_camera const* camera)
{
  SDL_assert(grid_width(&wr->terrain) == camera->bounds.w);
  SDL_assert(grid_height(&wr->terrain) == camera->bounds.h);

  grid(rl_tile) const* map = &rl_get_current_level(world)->map;

  wr->visible = rl_visible_world(camera, grid_width(map), grid_height(map));

  populate_terrain(wr, world, camera);
  populate_light(wr, world, camera);
}

void
rl_draw_world(struct rl_world_renderer const* wr,
            struct rl_world const* world,
            struct rl_camera const* camera,
            SDL_Renderer* renderer,
            struct gfx_tileset const* font)
{
  SDL_assert(font->tile_width == camera->cell_width);
  SDL_assert(font->tile_height == camera->cell_height);

  SDL_FPoint const screen_origin = rl_viewport_origin(camera);
  draw_level(wr, renderer, font, screen_origin);
  draw_items(camera, world, renderer, font);
  draw_actors(camera, world, renderer, font);
  draw_light(wr, renderer, font, screen_origin);
}

void
rl_draw_world_target_area(struct rl_world_renderer const* wr,
                        struct rl_camera const* camera,
                        SDL_Renderer* renderer,
                        SDL_Rect const* bounds,
                        grid(boolean) const* mask)
{
  SDL_Rect region;
  if (!SDL_GetRectIntersection(&wr->visible, bounds, &region)) {
    // nothing within the displayed grid
    return;
  }

  SDL_BlendMode prev;
  SDL_GetRenderDrawBlendMode(renderer, &prev);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_FColor const colour = RL_COLOUR_RED[5];
  SDL_SetRenderDrawColorFloat(renderer, colour.r, colour.g, colour.b, 0.35f);

  for (int y = region.y; y < region.y + region.h; y++) {
    for (int x = region.x; x < region.x + region.w; x++) {
      if (!*grid_at(mask, x - bounds->x, y - bounds->y)) {
        continue;
      }

      SDL_Point const p = { x, y };
      SDL_FPoint const at = rl_world_to_screen(camera, p);
      SDL_FRect const rect = {
        at.x, at.y, (float)camera->cell_width, (float)camera->cell_height
      };
      SDL_RenderFillRect(renderer, &rect);
    }
  }

  SDL_SetRenderDrawBlendMode(renderer, prev);
}

void
rl_draw_world_cursor(struct rl_camera const* camera,
                   SDL_Renderer* renderer,
                   SDL_Point cursor)
{
  SDL_FPoint const at = rl_world_to_screen(camera, cursor);
  SDL_FRect const rect = {
    at.x, at.y, (float)camera->cell_width, (float)camera->cell_height
  };

  SDL_FColor colour = RL_COLOUR_YELLOW[5];
  SDL_SetRenderDrawColorFloat(renderer, colour.r, colour.g, colour.b, colour.a);
  SDL_RenderRect(renderer, &rect);
}
