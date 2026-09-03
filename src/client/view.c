#include "view.h"

#include <SDL3/SDL_render.h>

#include "game/game_state.h"
#include "game/tile_map.h"

#include "client/client.h"
#include "client/graphics.h"
#include "client/lighting.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/resources.h"
#include "client/ui.h"

static void
draw_map(SDL_Renderer* renderer,
         SDL_Texture* font,
         grid(rl_tile) const* map,
         struct rl_fov const* fov)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  for (int y = 0; y < grid_height(map); y++) {
    for (int x = 0; x < grid_width(map); x++) {
      size_t const index = grid_index_of(map, x, y);

      if (!fov->explored.data[index]) {
        // don't draw anything for unexplored tiles
        continue;
      }

      enum rl_tile const tile = *grid_at(map, x, y);
      struct rl_gfx_tile gfx = rl_get_tile_gfx(tile);

      if (!fov->visible.data[index]) {
        // dim explored but not visible tiles
        gfx.fg = rl_lerp_colour(gfx.fg, RL_COLOUR_BLACK, 0.4f);
      }

      rl_draw_tile(renderer, font, &gfx, RL_UI_MAP_X + x, RL_UI_MAP_Y + y);
    }
  }
}

static void
draw_light(SDL_Renderer* renderer,
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
      rl_fill_tile(renderer, colour, RL_UI_MAP_X + x, RL_UI_MAP_Y + y);
    }
  }
}

static void
draw_actor(SDL_Renderer* renderer,
           SDL_Texture* font,
           struct rl_actor const* actor)
{
  struct rl_gfx_tile const tile = rl_get_actor_gfx(actor);
  rl_draw_tile(renderer,
               font,
               &tile,
               RL_UI_MAP_X + actor->pos.x,
               RL_UI_MAP_Y + actor->pos.y);
}

static void
draw_actors(SDL_Renderer* renderer,
            SDL_Texture* font,
            struct rl_world const* world,
            struct rl_fov const* fov)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  for (int i = 0; i < alist_len(&world->actors); i++) {
    struct rl_actor const* actor = alist_at(&world->actors, i);

    if (!rl_actor_is_alive(actor)) {
      continue;
    }

    size_t const index =
      grid_index_of(&world->map, actor->pos.x, actor->pos.y);

    if (fov->visible.data[index]) {
      draw_actor(renderer, font, actor);
    }
  }
}

static void
draw_game_log(SDL_Renderer* renderer,
              SDL_Texture* font,
              struct rl_game_log const* log)
{
  int row = RL_UI_BPANEL_Y;
  int const len = (int)alist_len(&log->messages);

  // no scrolling controls yet, so show only the latest messages
  int const start = SDL_max(0, len - RL_UI_BPANEL_HEIGHT);

  for (int i = start; i < len; i++) {
    struct rl_text const* message = alist_at(&log->messages, i);
    rl_draw_text(renderer, font, message, RL_UI_BPANEL_X, row);
    row += 1;
  }
}

static void
draw_side_panel(SDL_Renderer* renderer,
                SDL_Texture* font,
                struct rl_game_state const* game_state)
{
  struct rl_actor const* rogue =
    rl_get_actor(&game_state->world, RL_ROGUE_ID);

  char text[16];
  SDL_snprintf(text, sizeof(text), "HP: %d / %d", rogue->hp, rogue->max_hp);

  rl_draw_string(renderer,
                 font,
                 text,
                 RL_COLOUR_GRAY[5],
                 RL_COLOUR_BLACK,
                 RL_UI_RPANEL_X,
                 RL_UI_RPANEL_Y);
}

static void
draw_top_panel(SDL_Renderer* renderer, SDL_Texture* font)
{
  char const* text = "\x18 W | \x1B A | \x19 S | \x1A D";

  rl_draw_string(renderer,
                 font,
                 text,
                 RL_COLOUR_GRAY[5],
                 RL_COLOUR_BLACK,
                 RL_UI_TPANEL_X,
                 RL_UI_TPANEL_Y);
}

static void
draw_ui(SDL_Renderer* renderer,
        SDL_Texture* font,
        struct rl_client const* client)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  draw_game_log(renderer, font, &client->log);
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
  draw_actors(renderer,
              client->resources.font,
              &client->game_state.world,
              &client->game_state.fov);
  draw_light(renderer, &client->game_state.world.map, &client->game_state.fov);
  draw_ui(renderer, client->resources.font, client);
}
