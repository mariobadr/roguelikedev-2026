#include "game.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "fov.h"
#include "input.h"
#include "palette.h"
#include "render.h"
#include "resources.h"

#define MAP_WIDTH 40
#define MAP_HEIGHT 20
#define FOV_RADIUS 6

/**
 * Return the sign of number.
 *
 * @param number The value to check.
 *
 * @return -1 for negative, 1 for positive, and 0 for zero
 */
static int
sign(int number)
{
  return (number > 0) - (number < 0);
}

/**
 * Update action to a move action with (x, y) as the vector.
 *
 * @param action the action to mutate
 * @param x      the x-component of the vector
 * @param y      the y-component of the vector
 */
static void
set_movement_action(struct rl_action* action, int x, int y)
{
  action->type = RL_ACTION_MOVE;
  action->move_vector.x = x;
  action->move_vector.y = y;
}

static void
move_entity(struct rl_entity* entity, struct rl_map* map, SDL_Point move_vector)
{
  SDL_Point dst = { 0 };
  dst.x = entity->position.x + move_vector.x;
  dst.y = entity->position.y + move_vector.y;

  if (rl_is_walkable(rl_get_tile(map, dst.x, dst.y))) {
    entity->position = dst;
  }
}

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

static SDL_FColor
apply_brightness(SDL_FColor colour, SDL_FColor fade_toward, float brightness)
{
  brightness = SDL_clamp(brightness, 0.0f, 1.0f);

  return (SDL_FColor){
    .r = fade_toward.r + (colour.r - fade_toward.r) * brightness,
    .g = fade_toward.g + (colour.g - fade_toward.g) * brightness,
    .b = fade_toward.b + (colour.b - fade_toward.b) * brightness,
    .a = colour.a,
  };
}

static void
set_wall_gfx(struct rl_gfx_tile* gfx, bool visible, float brightness)
{
  gfx->glyph = '#';
  gfx->bg = RL_COLOUR_NONE;

  if (visible) {
    gfx->fg =
      apply_brightness(RL_COLOUR_SLATE_5, RL_COLOUR_SLATE_2, brightness);
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
      apply_brightness(RL_COLOUR_SLATE_3, RL_COLOUR_SLATE_0, brightness);
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

static float
calculate_brightness(SDL_Point source, SDL_Point tile, float radius)
{
  float const dx = (float)(tile.x - source.x);
  float const dy = (float)(tile.y - source.y);
  float const distance = SDL_sqrtf(dx * dx + dy * dy);

  return SDL_clamp(1.0f - distance / radius, 0.0f, 1.0f);
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

      float const brightness = calculate_brightness(
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

/**
 * Update action based on any pressed keys in istate.
 *
 * @param action the action to mutate
 * @param istate the current frame's input state
 *
 * @return whether action was mutated
 */
static bool
handle_keyboard_input(struct rl_action* action, struct inpt_state const* istate)
{
  if (inpt_is_down(istate->keys[SDL_SCANCODE_W])) {
    set_movement_action(action, 0, -1);
    return true;
  }

  if (inpt_is_down(istate->keys[SDL_SCANCODE_S])) {
    set_movement_action(action, 0, 1);
    return true;
  }

  if (inpt_is_down(istate->keys[SDL_SCANCODE_A])) {
    set_movement_action(action, -1, 0);
    return true;
  }

  if (inpt_is_down(istate->keys[SDL_SCANCODE_D])) {
    set_movement_action(action, 1, 0);
    return true;
  }

  return false;
}

/**
 * Update action based on mouse state.
 *
 * @param action the action to mutate
 * @param istate the current frame's input state
 *
 * @return whether action was mutated
 */
static bool
handle_mouse_input(struct rl_game* game, struct inpt_state const* istate)
{
  if (inpt_is_down(istate->mouse.buttons[SDL_BUTTON_LEFT])) {
    int const target_x =
      (int)SDL_floorf(istate->mouse.position.x / GLYPH_WIDTH);
    int const target_y =
      (int)SDL_floorf(istate->mouse.position.y / GLYPH_HEIGHT);

    int const delta_x = target_x - game->rogue.position.x;
    int const delta_y = target_y - game->rogue.position.y;

    if (delta_x == 0 && delta_y == 0) {
      // already at target
      return false;
    }

    // move along x- or y-axis, but not both
    if (SDL_abs(delta_x) > SDL_abs(delta_y)) {
      set_movement_action(&game->action, sign(delta_x), 0);
    } else {
      set_movement_action(&game->action, 0, sign(delta_y));
    }

    return true;
  }

  return false;
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
    rl_free_map(&game->map);
    return false;
  }

  game->explored = SDL_calloc(MAP_WIDTH * MAP_HEIGHT, sizeof(*game->explored));
  if (game->explored == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    SDL_free(game->visible);
    rl_free_map(&game->map);
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

bool
rl_handle_input(struct rl_game* game, struct inpt_state const* istate)
{
  // reset the action for this frame
  game->action.type = RL_ACTION_NONE;

  if (game->action_cooldown > 0.0f) {
    // still recovering from the last action
    return true;
  }

  // prioritize keyboard input over mouse input (?)
  if (handle_keyboard_input(&game->action, istate)) {
    return true;
  }

  handle_mouse_input(game, istate);

  return true;
}

void
rl_update_game(struct rl_game* game, float dt)
{
  if (game->action_cooldown > 0.0f) {
    game->action_cooldown -= dt;
  }

  if (game->action.type == RL_ACTION_MOVE) {
    move_entity(&game->rogue, &game->map, game->action.move_vector);
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
