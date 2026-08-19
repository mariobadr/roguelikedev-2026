#include "game.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "controls.h"
#include "fov.h"
#include "lighting.h"
#include "palette.h"
#include "render.h"
#include "resources.h"

#define MAP_WIDTH 40
#define MAP_HEIGHT 20
#define FOV_RADIUS 6

static void
update_fov(struct rl_game* game)
{
  rl_compute_fov(&game->map, game->rogue.position, FOV_RADIUS, game->visible);
  for (int i = 0; i < game->map.width * game->map.height; i++) {
    if (game->visible[i]) {
      game->explored[i] = true;
    }
  }
}

static void
set_wall_gfx(struct rl_gfx_tile* gfx, bool visible, float brightness)
{
  gfx->glyph = '#';
  gfx->bg = RL_COLOUR_NONE;

  if (visible) {
    gfx->fg =
      rl_apply_brightness(RL_COLOUR_SLATE_5, RL_COLOUR_SLATE_2, brightness);
  } else {
    gfx->fg = RL_COLOUR_SLATE_2;
  }
}

static void
set_floor_gfx(struct rl_gfx_tile* gfx, bool visible, float brightness)
{
  gfx->glyph = ' ';
  gfx->fg = RL_COLOUR_NONE;

  if (visible) {
    gfx->bg =
      rl_apply_brightness(RL_COLOUR_SLATE_3, RL_COLOUR_SLATE_0, brightness);
  } else {
    gfx->bg = RL_COLOUR_NONE;
  }
}

static void
set_tile_gfx(struct rl_gfx_tile* gfx,
             struct rl_tile tile,
             bool visible,
             float brightness)
{
  switch (tile.type) {
    case RL_TILE_WALL:
      set_wall_gfx(gfx, visible, brightness);
      break;
    case RL_TILE_FLOOR:
      set_floor_gfx(gfx, visible, brightness);
      break;
    default:
      gfx->glyph = ' ';
      gfx->bg = RL_COLOUR_NONE;
      gfx->fg = RL_COLOUR_NONE;
      break;
  }
}

static void
draw_map(SDL_Renderer* renderer, SDL_Texture* font, struct rl_game* game)
{
  struct rl_gfx_tile gfx = { 0 };

  for (int y = 0; y < game->map.height; y++) {
    for (int x = 0; x < game->map.width; x++) {
      size_t const index = rl_map_index_of(&game->map, x, y);

      if (!game->explored[index]) {
        continue;
      }

      float const brightness = rl_calculate_brightness(
        game->rogue.position, (SDL_Point){ x, y }, FOV_RADIUS);
      struct rl_tile const tile = rl_get_tile(&game->map, x, y);

      set_tile_gfx(&gfx, tile, game->visible[index], brightness);
      rl_draw_tile(renderer, font, &gfx, x * GLYPH_WIDTH, y * GLYPH_HEIGHT);
    }
  }
}

static void
draw_entity(SDL_Renderer* renderer,
            SDL_Texture* font,
            struct rl_entity const* entity)
{
  struct rl_gfx_tile tile = { 0 };
  tile.glyph = entity->glyph;
  tile.fg = RL_COLOUR_WHITE;
  tile.bg = RL_COLOUR_NONE;

  // convert from tile to screen coordinates
  float const x = entity->position.x * GLYPH_WIDTH;
  float const y = entity->position.y * GLYPH_HEIGHT;

  rl_draw_tile(renderer, font, &tile, x, y);
}

bool
rl_init_game(struct rl_game* game, struct rl_resources const* resources)
{
  if (!rl_init_map(&game->map, MAP_WIDTH, MAP_HEIGHT)) {
    return false;
  }

  game->visible = SDL_calloc(MAP_WIDTH * MAP_HEIGHT, sizeof(*game->visible));
  if (game->visible == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    rl_free_game(game);
    return false;
  }

  game->explored = SDL_calloc(MAP_WIDTH * MAP_HEIGHT, sizeof(*game->explored));
  if (game->explored == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    rl_free_game(game);
    return false;
  }

  game->resources = resources;

  game->rogue.glyph = '@';

  // just put the rogue at the centre of the first room
  SDL_Rect const* room = &game->map.rooms[0];
  game->rogue.position.x = room->x + room->w / 2;
  game->rogue.position.y = room->y + room->h / 2;
  // and make sure we have an initial field-of-view
  update_fov(game);

  game->action.type = RL_ACTION_NONE;
  game->action_cooldown = 0.0f;

  return true;
}

void
rl_free_game(struct rl_game* game)
{
  if (game == NULL) {
    return;
  }

  SDL_free(game->visible);
  game->visible = NULL;

  SDL_free(game->explored);
  game->explored = NULL;

  rl_free_map(&game->map);
}

bool
rl_handle_input(struct rl_game* game, struct inpt_state const* istate)
{
  // reset the action for this frame
  game->action.type = RL_ACTION_NONE;

  if (game->action_cooldown > 0.0f) {
    // still recovering from the last action
    return true;
  }

  game->action = rl_translate_input(istate, game->rogue.position);

  return true;
}

void
rl_update_game(struct rl_game* game, float dt)
{
  if (game->action_cooldown > 0.0f) {
    game->action_cooldown -= dt;
  }

  if (game->action.type == RL_ACTION_MOVE) {
    rl_move_entity(&game->rogue, &game->map, game->action.move_vector);
    game->action_cooldown = 0.115f;

    // the rogue has moved, update the field-of-view
    update_fov(game);
  }
}

void
rl_render_game(struct rl_game* game, SDL_Renderer* renderer)
{
  SDL_SetRenderDrawColorFloat(renderer,
                              RL_COLOUR_SLATE_0.r,
                              RL_COLOUR_SLATE_0.g,
                              RL_COLOUR_SLATE_0.b,
                              RL_COLOUR_SLATE_0.a);
  SDL_RenderClear(renderer);

  draw_map(renderer, game->resources->font, game);
  draw_entity(renderer, game->resources->font, &game->rogue);
}
