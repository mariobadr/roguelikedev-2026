#include "game.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "controls.h"
#include "graphics.h"
#include "lighting.h"
#include "palette.h"
#include "render.h"
#include "resources.h"

#define FOV_RADIUS 8

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

      // convert from tile to screen coordinates
      float const fx = x * GLYPH_WIDTH;
      float const fy = y * GLYPH_HEIGHT;

      rl_draw_tile(renderer, font, &gfx, fx, fy);
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

      // convert from tile to screen coordinates
      float const fx = x * GLYPH_WIDTH;
      float const fy = y * GLYPH_HEIGHT;
      SDL_FRect const cell = { fx, fy, GLYPH_WIDTH, GLYPH_HEIGHT };

      SDL_SetRenderDrawColorFloat(renderer, light.r, light.g, light.b, alpha);
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

  // convert from tile to screen coordinates
  float const fx = entity->position.x * GLYPH_WIDTH;
  float const fy = entity->position.y * GLYPH_HEIGHT;

  rl_draw_tile(renderer, font, &tile, fx, fy);
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
      rl_map_index_of(&world->map, entity->position.x, entity->position.y);

    if (fov->visible.data[index]) {
      draw_entity(renderer, font, entity);
    }
  }
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

  struct rl_entity const* rogue = rl_get_entity(&game->world, RL_ROGUE_ID);
  rl_update_fov(&game->fov, &game->world.map, rogue->position);

  return true;
}

bool
rl_init_game(struct rl_game* game, struct rl_resources const* resources)
{
  rand_seed(&game->rng, 1234);

  if (!alist_alloc(&game->events, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    return false;
  }

  if (!rl_init_world(&game->world, MAP_WIDTH, MAP_HEIGHT, &game->rng)) {
    rl_free_game(game);
    return false;
  }

  if (!rl_init_fov(&game->fov, MAP_WIDTH * MAP_HEIGHT, FOV_RADIUS)) {
    rl_free_game(game);
    return false;
  }

  // make sure the rogue has an initial field-of-view
  struct rl_entity const* rogue = rl_get_entity(&game->world, RL_ROGUE_ID);
  rl_update_fov(&game->fov, &game->world.map, rogue->position);

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
  alist_free(&game->events);
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

  struct rl_entity const* rogue = rl_get_entity(&game->world, RL_ROGUE_ID);
  game->action = rl_translate_input(istate, rogue->position);

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

  // Build a command based on the player's last action
  struct rl_command cmd = rl_build_command(game->action);

  // Do the simulation
  if (rl_update_world(&game->world, &cmd, &game->events, &game->rng)) {
    // turn taken
    game->action_cooldown = ACTION_GLOBAL_COOLDOWN;
  }

  struct rl_entity const* rogue = rl_get_entity(&game->world, RL_ROGUE_ID);
  for (int i = 0; i < alist_len(&game->events); i++) {
    struct rl_event const* event = alist_at(&game->events, i);

    switch (event->type) {
      case RL_EVENT_MOVE:
        rl_update_fov(&game->fov, &game->world.map, rogue->position);
        break;
      case RL_EVENT_ATTACK:
        SDL_Log("Attack: %d, %d, %d",
                event->as.attack.attacker,
                event->as.attack.defender,
                event->as.attack.damage);
        break;
      case RL_EVENT_DEATH:
        SDL_Log(
          "Death: %d, %d", event->as.death.entity, event->as.death.killer);
        break;
      default:
        break;
    }
  }

  alist_clear(&game->events);
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

  draw_map(renderer, game->resources->font, &game->world.map, &game->fov);
  draw_entities(renderer, game->resources->font, &game->world, &game->fov);
  draw_light(renderer, &game->world.map, &game->fov);
}
