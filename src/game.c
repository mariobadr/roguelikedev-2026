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
#include "ui.h"

#define FOV_RADIUS 8

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
  SDL_FPoint const position = tile_to_screen(
    RL_UI_MAP_X + entity->position.x, RL_UI_MAP_Y + entity->position.y);

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
      rl_map_index_of(&world->map, entity->position.x, entity->position.y);

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
                struct rl_game const* game)
{
  struct rl_entity const* rogue = rl_get_entity(&game->world, RL_ROGUE_ID);

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
draw_ui(SDL_Renderer* renderer, SDL_Texture* font, struct rl_game const* game)
{
  draw_combat_log(renderer, font, &game->messages);
  draw_side_panel(renderer, font, game);
  draw_top_panel(renderer, font);
}

/**
 * TODO: fix all the duplication between here and rl_init_game
 */
static bool
regenerate_map(struct rl_game* game)
{
  struct rl_world new_world = { 0 };
  if (!rl_init_world(
        &new_world, RL_UI_MAP_WIDTH, RL_UI_MAP_HEIGHT, &game->rng)) {
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

  if (!alist_alloc(&game->messages, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    rl_free_game(game);
    return false;
  }

  if (!rl_init_world(
        &game->world, RL_UI_MAP_WIDTH, RL_UI_MAP_HEIGHT, &game->rng)) {
    rl_free_game(game);
    return false;
  }

  if (!rl_init_fov(
        &game->fov, RL_UI_MAP_WIDTH * RL_UI_MAP_HEIGHT, FOV_RADIUS)) {
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
  alist_free(&game->messages);
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
        *alist_push(&game->messages) =
          rl_build_attack_log(&game->world, &event->as.attack);
        break;
      case RL_EVENT_DEATH:
        *alist_push(&game->messages) =
          rl_build_death_log(&game->world, &event->as.death);
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
  draw_ui(renderer, game->resources->font, game);
}
