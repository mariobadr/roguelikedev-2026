#include "view.h"

#include <SDL3/SDL_render.h>

#include "game/game_state.h"

#include "client/graphics.h"
#include "client/lighting.h"
#include "client/log.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/resources.h"
#include "client/client.h"
#include "client/ui.h"

static SDL_FPoint
tile_to_screen(int x, int y)
{
  return (SDL_FPoint){
    .x = x * GLYPH_WIDTH,
    .y = y * GLYPH_HEIGHT,
  };
}

static void
draw_map(SDL_Renderer* renderer,
         SDL_Texture* font,
         struct rl_tile_map const* map,
         struct rl_fov const* fov)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  for (int y = 0; y < map->height; y++) {
    for (int x = 0; x < map->width; x++) {
      size_t const index = rl_map_index_of(map, x, y);

      if (!fov->explored.data[index]) {
        // don't draw anything for unexplored tiles
        continue;
      }

      enum rl_tile const tile = rl_get_tile(map, x, y);
      struct rl_gfx_tile gfx = rl_get_tile_gfx(tile);

      if (!fov->visible.data[index]) {
        // dim explored but not visible tiles
        gfx.fg = rl_lerp_colour(gfx.fg, RL_COLOUR_BLACK, 0.4f);
      }

      SDL_FPoint const position =
        tile_to_screen(RL_UI_MAP_X + x, RL_UI_MAP_Y + y);

      rl_draw_tile(renderer, font, &gfx, position.x, position.y);
    }
  }
}

static void
draw_light(SDL_Renderer* renderer,
           struct rl_tile_map const* map,
           struct rl_fov const* fov)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

  // the colour of the light source - make this an argument?
  SDL_FColor const light = RL_COLOUR_GRAY[6];

  for (int y = 0; y < map->height; y++) {
    for (int x = 0; x < map->width; x++) {
      size_t const index = rl_map_index_of(map, x, y);

      if (!fov->visible.data[index]) {
        // not visible, so there's no "glow" to add
        continue;
      }

      float const brightness = rl_calculate_brightness(
        fov->origin, (SDL_Point){ x, y }, (float)fov->radius);
      float const alpha = rl_lerp_float(0.6f, 0.0f, brightness);

      SDL_FPoint const position =
        tile_to_screen(RL_UI_MAP_X + x, RL_UI_MAP_Y + y);

      SDL_SetRenderDrawColorFloat(renderer, light.r, light.g, light.b, alpha);

      SDL_FRect const cell = {
        position.x, position.y, GLYPH_WIDTH, GLYPH_HEIGHT
      };
      SDL_RenderFillRect(renderer, &cell);
    }
  }
}

static void
draw_entity(SDL_Renderer* renderer,
            SDL_Texture* font,
            struct rl_entity const* entity)
{
  struct rl_gfx_tile const tile = rl_get_entity_gfx(entity);
  SDL_FPoint const position = tile_to_screen(RL_UI_MAP_X + entity->pos.x,
                                             RL_UI_MAP_Y + entity->pos.y);

  rl_draw_tile(renderer, font, &tile, position.x, position.y);
}

static void
draw_entities(SDL_Renderer* renderer,
              SDL_Texture* font,
              struct rl_world const* world,
              struct rl_fov const* fov)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  for (int i = 0; i < alist_len(&world->entities); i++) {
    struct rl_entity const* entity = alist_at(&world->entities, i);

    if (!rl_entity_is_alive(entity)) {
      continue;
    }

    size_t const index =
      rl_map_index_of(&world->map, entity->pos.x, entity->pos.y);

    if (fov->visible.data[index]) {
      draw_entity(renderer, font, entity);
    }
  }
}

static void
draw_text(SDL_Renderer* renderer,
          SDL_Texture* font,
          char const* text,
          int x,
          int y,
          size_t max_width,
          SDL_FColor colour)
{
  float const fx = x * GLYPH_WIDTH;
  float const fy = y * GLYPH_HEIGHT;

  struct rl_gfx_tile tile = { 0 };
  tile.fg = colour;
  tile.bg = RL_COLOUR_BLACK;

  size_t const length = SDL_min(SDL_strlen(text), max_width);
  for (size_t i = 0; i < length; i++) {
    tile.glyph = text[i];
    rl_draw_tile(renderer, font, &tile, fx + i * GLYPH_WIDTH, fy);
  }
}

static void
draw_combat_log(SDL_Renderer* renderer,
                SDL_Texture* font,
                alist(rl_log_line) const* messages)
{
  float const fx = RL_UI_BPANEL_X * GLYPH_WIDTH;
  float fy = RL_UI_BPANEL_Y * GLYPH_HEIGHT;

  int const len = (int)alist_len(messages);

  // no scrolling controls yet, so show only the latest messages
  int const start = SDL_max(0, len - RL_UI_BPANEL_HEIGHT);

  struct rl_gfx_tile tile = { 0 };
  tile.bg = RL_COLOUR_BLACK;

  for (int i = start; i < len; i++) {
    struct rl_log_line const* line = alist_at(messages, i);

    // truncate line, just in case (no word wrap yet)
    int const width = SDL_min(line->len, RL_UI_MAP_WIDTH);
    for (int j = 0; j < width; j++) {
      tile.glyph = line->msg[j].glyph;
      tile.fg = line->msg[j].fg;
      rl_draw_tile(renderer, font, &tile, fx + j * GLYPH_WIDTH, fy);
    }

    fy += GLYPH_HEIGHT;
  }
}

static void
draw_side_panel(SDL_Renderer* renderer,
                SDL_Texture* font,
                struct rl_game_state const* game_state)
{
  struct rl_entity const* rogue =
    rl_get_entity(&game_state->world, RL_ROGUE_ID);

  char text[16];
  SDL_snprintf(text, sizeof(text), "HP: %d / %d", rogue->hp, rogue->max_hp);

  draw_text(renderer,
            font,
            text,
            RL_UI_RPANEL_X,
            RL_UI_RPANEL_Y,
            RL_UI_RPANEL_WIDTH,
            RL_COLOUR_GRAY[5]);
}

static void
draw_top_panel(SDL_Renderer* renderer, SDL_Texture* font)
{
  const char* text = "\x18 W | \x1B A | \x19 S | \x1A D";

  draw_text(renderer,
            font,
            text,
            RL_UI_TPANEL_X,
            RL_UI_TPANEL_Y,
            RL_UI_TPANEL_WIDTH,
            RL_COLOUR_GRAY[5]);
}

static void
draw_ui(SDL_Renderer* renderer, SDL_Texture* font, struct rl_client const* client)
{
  draw_combat_log(renderer, font, &client->messages);
  draw_side_panel(renderer, font, &client->game_state);
  draw_top_panel(renderer, font);
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

  draw_map(renderer,
           client->resources.font,
           &client->game_state.world.map,
           &client->game_state.fov);
  draw_entities(renderer,
                client->resources.font,
                &client->game_state.world,
                &client->game_state.fov);
  draw_light(renderer,
             &client->game_state.world.map,
             &client->game_state.fov);
  draw_ui(renderer, client->resources.font, client);
}
