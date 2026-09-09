#include "view.h"

#include <SDL3/SDL_render.h>

#include "game/game_state.h"
#include "game/tile.h"

#include "client/cell.h"
#include "client/client.h"
#include "client/font.h"
#include "client/graphics.h"
#include "client/lighting.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/ui.h"

static SDL_Rect
panel_clip_rect(SDL_Rect const* panel)
{
  SDL_FRect const px = rl_cell_rect_to_pixels(panel);
  return (SDL_Rect){ (int)px.x, (int)px.y, (int)px.w, (int)px.h };
}

static void
draw_level(SDL_Renderer* renderer,
           struct rl_font const* font,
           SDL_Rect const* panel,
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

      SDL_Point const at = rl_translate_ui_position(panel, x, y);
      rl_draw_tile(renderer, font, &gfx, at.x, at.y);
    }
  }
}

static void
draw_light(SDL_Renderer* renderer,
           SDL_Rect const* panel,
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
      SDL_Point const at = rl_translate_ui_position(panel, x, y);
      rl_fill_tile(renderer, colour, at.x, at.y);
    }
  }
}

static void
draw_item(SDL_Renderer* renderer,
          struct rl_font const* font,
          SDL_Rect const* panel,
          struct rl_item const* item)
{
  struct rl_gfx_tile const tile = rl_get_item_gfx(item);
  SDL_Point const at =
    rl_translate_ui_position(panel, item->on.map.x, item->on.map.y);
  rl_draw_tile(renderer, font, &tile, at.x, at.y);
}

static void
draw_items(SDL_Renderer* renderer,
           struct rl_font const* font,
           SDL_Rect const* panel,
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
           SDL_Rect const* panel,
           struct rl_actor const* actor)
{
  struct rl_gfx_tile const tile = rl_get_actor_gfx(actor);
  SDL_Point const at =
    rl_translate_ui_position(panel, actor->pos.x, actor->pos.y);
  rl_draw_tile(renderer, font, &tile, at.x, at.y);
}

static void
draw_actors(SDL_Renderer* renderer,
            struct rl_font const* font,
            SDL_Rect const* panel,
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

static void
draw_game_log(SDL_Renderer* renderer,
              struct rl_font const* font,
              SDL_Rect const* panel,
              struct rl_game_log const* log)
{
  int row = 0;
  int const len = (int)alist_len(&log->messages);

  // no scrolling controls yet, so show only the latest messages
  int const start = SDL_max(0, len - panel->h);

  for (int i = start; i < len; i++) {
    struct rl_text const* message = alist_at(&log->messages, i);
    SDL_Point const at = rl_translate_ui_position(panel, 0, row);
    rl_draw_text(renderer, font, message, at.x, at.y);
    row += 1;
  }
}

static void
draw_side_panel(SDL_Renderer* renderer,
                struct rl_font const* font,
                SDL_Rect const* panel,
                struct rl_game_state const* game_state)
{
  struct rl_actor const* rogue = rl_get_actor(&game_state->world, RL_ROGUE_ID);

  char text[16];
  SDL_snprintf(text, sizeof(text), "HP: %d / %d", rogue->hp, rogue->max_hp);

  SDL_Point const at = rl_translate_ui_position(panel, 0, 0);
  rl_draw_string(
    renderer, font, text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at.x, at.y);
}

static void
draw_top_panel(SDL_Renderer* renderer,
               struct rl_font const* font,
               SDL_Rect const* panel)
{
  char const* text = "\x18 W | \x1B A | \x19 S | \x1A D";

  SDL_Point const at = rl_translate_ui_position(panel, 0, 0);
  rl_draw_string(
    renderer, font, text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at.x, at.y);
}

static void
draw_ui(SDL_Renderer* renderer,
        struct rl_font const* font,
        struct rl_client const* client)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_Rect const bottom_clip = panel_clip_rect(&client->layout.bottom_panel);
  SDL_SetRenderClipRect(renderer, &bottom_clip);
  draw_game_log(renderer, font, &client->layout.bottom_panel, &client->log);
  SDL_SetRenderClipRect(renderer, NULL);

  SDL_Rect const right_clip = panel_clip_rect(&client->layout.right_panel);
  SDL_SetRenderClipRect(renderer, &right_clip);
  draw_side_panel(
    renderer, font, &client->layout.right_panel, &client->game_state);
  SDL_SetRenderClipRect(renderer, NULL);

  SDL_Rect const top_clip = panel_clip_rect(&client->layout.top_panel);
  SDL_SetRenderClipRect(renderer, &top_clip);
  draw_top_panel(renderer, font, &client->layout.top_panel);
  SDL_SetRenderClipRect(renderer, NULL);
}

void
rl_render_game(SDL_Renderer* renderer, struct rl_client* client)
{
  SDL_SetRenderDrawColorFloat(renderer,
                              RL_COLOUR_GRAY[9].r,
                              RL_COLOUR_GRAY[9].g,
                              RL_COLOUR_GRAY[9].b,
                              RL_COLOUR_GRAY[9].a);
  SDL_RenderClear(renderer);

  struct rl_level const* level =
    rl_get_current_level(&client->game_state.world);
  SDL_Rect const* main_panel = &client->layout.main_panel;

  SDL_Rect const main_clip = panel_clip_rect(main_panel);
  SDL_SetRenderClipRect(renderer, &main_clip);
  draw_level(
    renderer, &client->font, main_panel, level, &client->game_state.fov);
  draw_items(renderer,
             &client->font,
             main_panel,
             &client->game_state.world,
             &client->game_state.fov);
  draw_actors(renderer,
              &client->font,
              main_panel,
              &client->game_state.world,
              &client->game_state.fov);
  draw_light(renderer, main_panel, &level->map, &client->game_state.fov);
  SDL_SetRenderClipRect(renderer, NULL);

  draw_ui(renderer, &client->font, client);
}
