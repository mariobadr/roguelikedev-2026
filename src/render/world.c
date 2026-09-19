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

#include "graphics/camera.h"
#include "graphics/grid_view.h"
#include "graphics/tileset.h"

#include "graphics.h"
#include "lighting.h"
#include "palette.h"

static SDL_Point
map_to_buffer(struct rl_world_renderer const* wr, SDL_Point cell)
{
  return (SDL_Point){ cell.x - wr->bounds.x, cell.y - wr->bounds.y };
}

static void
populate_terrain(struct rl_world_renderer* wr, struct rl_world const* world)
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

      SDL_Point const local = map_to_buffer(wr, p);
      *grid_at(&wr->terrain, local.x, local.y) = cell;
    }
  }
}

static void
populate_light(struct rl_world_renderer* wr, struct rl_world const* world)
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

      SDL_Point const local = map_to_buffer(wr, p);
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
draw_item(struct gfx_camera const* camera,
          struct gfx_grid_view const* view,
          SDL_Renderer* renderer,
          struct gfx_tileset const* font,
          struct rl_item const* item)
{
  struct gfx_console_cell const cell = rl_get_item_gfx(item);
  SDL_FPoint const at = gfx_cell_to_screen(view, camera, item->on.map);
  SDL_FRect dst = gfx_tileset_dst(font, at, 1);
  gfx_draw_console_cell(renderer, font, &cell, &dst);
}

static void
draw_items(struct gfx_camera const* camera,
           struct gfx_grid_view const* view,
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
      draw_item(camera, view, renderer, font, item);
    }
  }
}

static void
draw_actor(struct gfx_camera const* camera,
           struct gfx_grid_view const* view,
           SDL_Renderer* renderer,
           struct gfx_tileset const* font,
           struct rl_actor const* actor)
{
  struct gfx_console_cell const cell = rl_get_actor_gfx(actor);
  SDL_FPoint const at = gfx_cell_to_screen(view, camera, actor->pos);
  SDL_FRect dst = gfx_tileset_dst(font, at, 1);
  gfx_draw_console_cell(renderer, font, &cell, &dst);
}

static void
draw_actors(struct gfx_camera const* camera,
            struct gfx_grid_view const* view,
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
      draw_actor(camera, view, renderer, font, actor);
    }
  }
}

bool
rl_init_world_renderer(struct rl_world_renderer* wr, int columns, int rows)
{
  wr->bounds = (SDL_Rect){ 0 };
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
                          struct gfx_camera const* camera,
                          struct gfx_grid_view const* view)
{
  wr->bounds = gfx_grid_view_bounds(view, camera);
  SDL_assert(grid_width(&wr->terrain) == wr->bounds.w);
  SDL_assert(grid_height(&wr->terrain) == wr->bounds.h);

  grid(rl_tile) const* map = &rl_get_current_level(world)->map;

  wr->visible =
    gfx_visible_cells(view, camera, grid_width(map), grid_height(map));

  populate_terrain(wr, world);
  populate_light(wr, world);
}

void
rl_draw_world(struct rl_world_renderer const* wr,
              struct rl_world const* world,
              struct gfx_camera const* camera,
              struct gfx_grid_view const* view,
              SDL_Renderer* renderer,
              struct gfx_tileset const* font)
{
  SDL_assert(font->tile_width == view->cell_width);
  SDL_assert(font->tile_height == view->cell_height);

  SDL_Point const first = { wr->bounds.x, wr->bounds.y };
  SDL_FPoint const screen_origin = gfx_cell_to_screen(view, camera, first);
  draw_level(wr, renderer, font, screen_origin);
  draw_items(camera, view, world, renderer, font);
  draw_actors(camera, view, world, renderer, font);
  draw_light(wr, renderer, font, screen_origin);
}

void
rl_draw_world_target_area(struct rl_world_renderer const* wr,
                          struct gfx_camera const* camera,
                          struct gfx_grid_view const* view,
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
      SDL_FPoint const at = gfx_cell_to_screen(view, camera, p);
      SDL_FRect const rect = {
        at.x, at.y, (float)view->cell_width, (float)view->cell_height
      };
      SDL_RenderFillRect(renderer, &rect);
    }
  }

  SDL_SetRenderDrawBlendMode(renderer, prev);
}

void
rl_draw_world_cursor(struct gfx_camera const* camera,
                     struct gfx_grid_view const* view,
                     SDL_Renderer* renderer,
                     SDL_Point cursor)
{
  SDL_FPoint const at = gfx_cell_to_screen(view, camera, cursor);
  SDL_FRect const rect = {
    at.x, at.y, (float)view->cell_width, (float)view->cell_height
  };

  SDL_FColor colour = RL_COLOUR_YELLOW[5];
  SDL_SetRenderDrawColorFloat(renderer, colour.r, colour.g, colour.b, colour.a);
  SDL_RenderRect(renderer, &rect);
}
