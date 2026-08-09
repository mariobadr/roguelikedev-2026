#include "game.h"

#include <SDL3/SDL_render.h>

#include "input.h"
#include "render.h"
#include "resources.h"

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
move_entity(struct rl_entity* entity,
            struct rl_world_map* map,
            SDL_Point move_vector)
{
  SDL_Point dst = { 0 };
  dst.x = entity->position.x + move_vector.x;
  dst.y = entity->position.y + move_vector.y;

  if (rl_get_tile(map, dst.x, dst.y).walkable) {
    entity->position = dst;
  }
}

static void
draw_map(SDL_Renderer* renderer,
         SDL_Texture* font,
         struct rl_world_map const* map)
{
  struct rl_gfx_tile wall = { 0 };
  wall.glyph = '#';
  wall.fg = RL_COLOUR_WHITE;
  wall.bg = RL_COLOUR_NONE;

  for (int y = 0; y < map->height; y++) {
    for (int x = 0; x < map->width; x++) {
      if (!rl_get_tile(map, x, y).walkable) {
        rl_draw_tile(renderer, font, &wall, x * GLYPH_WIDTH, y * GLYPH_HEIGHT);
      }
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
  if (!rl_init_map(&game->map, 40, 20)) {
    return false;
  }

  game->resources = resources;

  game->rogue.glyph = '@';
  game->rogue.position.x = 0;
  game->rogue.position.y = 0;

  game->action.type = RL_ACTION_NONE;
  game->action_cooldown = 0.0f;

  rl_generate_map(&game->map);

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
    // game->player.x += game->action.move_vector.x;
    // game->player.y += game->action.move_vector.y;
    game->action_cooldown = 0.115f;
  }
}

void
rl_render_game(struct rl_game* game, SDL_Renderer* renderer)
{
  SDL_SetRenderDrawColor(renderer, 16, 16, 16, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  draw_map(renderer, game->resources->font, &game->map);
  draw_entity(renderer, game->resources->font, &game->rogue);
}
