#include "game.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "controls.h"
#include "lighting.h"
#include "palette.h"
#include "render.h"
#include "resources.h"

#define FOV_RADIUS 5

static void
set_wall_gfx(struct rl_gfx_tile* gfx, bool visible, float brightness)
{
  gfx->glyph = '#';
  gfx->bg = RL_COLOUR_NONE;
  SDL_FColor const* fov_shades = RL_COLOUR_GRAY;

  if (visible) {
    gfx->fg = rl_apply_brightness(fov_shades[4], fov_shades[8], brightness);
  } else {
    gfx->fg = RL_COLOUR_GRAY[8];
  }
}

static void
set_floor_gfx(struct rl_gfx_tile* gfx, bool visible, float brightness)
{
  gfx->glyph = ' ';
  gfx->fg = RL_COLOUR_NONE;
  SDL_FColor const* fov_shades = RL_COLOUR_GRAY;

  if (visible) {
    gfx->bg = rl_apply_brightness(fov_shades[6], fov_shades[9], brightness);
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
draw_map(SDL_Renderer* renderer, SDL_Texture* font, struct rl_game const* game)
{
  struct rl_gfx_tile gfx = { 0 };

  for (int y = 0; y < game->world.map.height; y++) {
    for (int x = 0; x < game->world.map.width; x++) {
      size_t const index = rl_map_index_of(&game->world.map, x, y);

      if (!game->fov.explored[index]) {
        continue;
      }

      float const brightness = rl_calculate_brightness(
        game->world.rogue.position, (SDL_Point){ x, y }, FOV_RADIUS);
      struct rl_tile const tile = rl_get_tile(&game->world.map, x, y);

      set_tile_gfx(&gfx, tile, game->fov.visible[index], brightness);
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

/**
 * TODO: fix all the duplication between here and rl_init_game
 */
static bool
regenerate_map(struct rl_game* game)
{
  struct rl_world new_world = { 0 };
  if (!rl_init_world(&new_world, MAP_WIDTH, MAP_HEIGHT, &game->rng)) {
    return false;
  }

  rl_free_world(&game->world);
  game->world = new_world;

  rl_clear_fov(&game->fov);
  rl_update_fov(&game->fov, &game->world.map, game->world.rogue.position);

  return true;
}

bool
rl_init_game(struct rl_game* game, struct rl_resources const* resources)
{
  rand_seed(&game->rng, 1234);

  if (!rl_init_world(&game->world, MAP_WIDTH, MAP_HEIGHT, &game->rng)) {
    return false;
  }

  if (!rl_init_fov(&game->fov, MAP_WIDTH * MAP_HEIGHT, FOV_RADIUS)) {
    rl_free_game(game);
    return false;
  }

  // make sure the rogue has an initial field-of-view
  rl_update_fov(&game->fov, &game->world.map, game->world.rogue.position);

  game->resources = resources;
  game->action = RL_ACTION_NONE;
  game->action_cooldown = 0.0f;

  return true;
}

void
rl_free_game(struct rl_game* game)
{
  if (game == NULL) {
    return;
  }

  rl_free_fov(&game->fov);
  rl_free_world(&game->world);
}

bool
rl_handle_input(struct rl_game* game, struct inpt_state const* istate)
{
  // reset the action for this frame
  game->action = RL_ACTION_NONE;

  if (game->action_cooldown > 0.0f) {
    // still recovering from the last action
    return true;
  }

  game->action = rl_translate_input(istate, game->world.rogue.position);

  return true;
}

void
rl_update_game(struct rl_game* game, float dt)
{
  if (game->action_cooldown > 0.0f) {
    game->action_cooldown -= dt;
  }

  if (game->action == RL_ACTION_NONE) {
    return;
  } else if (game->action == RL_ACTION_DEBUG_GENMAP) {
    if (regenerate_map(game)) {
      game->action_cooldown = ACTION_GLOBAL_COOLDOWN;
    }

    return;
  }

  SDL_Point const prev = game->world.rogue.position;
  struct rl_command cmd = rl_build_command(game->action);
  rl_update_world(&game->world, &cmd);

  if (prev.x != game->world.rogue.position.x ||
      prev.y != game->world.rogue.position.y) {
    // the rogue moved
    rl_update_fov(&game->fov, &game->world.map, game->world.rogue.position);

    game->action_cooldown = ACTION_GLOBAL_COOLDOWN;
  }
}

void
rl_render_game(struct rl_game* game, SDL_Renderer* renderer)
{
  SDL_SetRenderDrawColorFloat(renderer,
                              RL_COLOUR_GRAY[9].r,
                              RL_COLOUR_GRAY[9].g,
                              RL_COLOUR_GRAY[9].b,
                              RL_COLOUR_GRAY[9].a);
  SDL_RenderClear(renderer);

  draw_map(renderer, game->resources->font, game);
  draw_entity(renderer, game->resources->font, &game->world.rogue);
}
