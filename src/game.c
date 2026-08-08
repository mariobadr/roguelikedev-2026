#include "game.h"

#include <SDL3/SDL_render.h>

#include "input.h"
#include "resources.h"

#define GLYPH_WIDTH (6.0f)
#define GLYPH_HEIGHT (8.0f)
#define FONT_ROWS (16)
#define FONT_COLS (16)

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

    int const delta_x = target_x - game->player.x;
    int const delta_y = target_y - game->player.y;

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

static void
render_glyph(SDL_Renderer* renderer,
             SDL_Texture* font,
             char glyph,
             float x,
             float y)
{
  // get a 0 to 255 index into the font
  unsigned char const index = glyph;

  SDL_FRect src = { 0 };
  src.x = (index % FONT_COLS) * GLYPH_WIDTH;
  src.y = (index / FONT_COLS) * GLYPH_HEIGHT;
  src.w = GLYPH_WIDTH;
  src.h = GLYPH_HEIGHT;

  SDL_FRect dst = { 0 };
  dst.x = x;
  dst.y = y;
  dst.w = GLYPH_WIDTH;
  dst.h = GLYPH_HEIGHT;

  SDL_RenderTexture(renderer, font, &src, &dst);
}

bool
rl_init_game(struct rl_game* game, struct rl_resources const* resources)
{
  game->resources = resources;

  game->player.x = 0;
  game->player.y = 0;

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
    game->player.x += game->action.move_vector.x;
    game->player.y += game->action.move_vector.y;
    game->action_cooldown = 0.115f;
  }
}

void
rl_render_game(struct rl_game* game, SDL_Renderer* renderer)
{
  SDL_SetRenderDrawColor(renderer, 16, 16, 16, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  // convert from tile to screen coordinates
  float const player_x = game->player.x * GLYPH_WIDTH;
  float const player_y = game->player.y * GLYPH_HEIGHT;

  // draw the rogue
  SDL_SetTextureColorMod(game->resources->font, 255, 255, 255);
  render_glyph(renderer, game->resources->font, '@', player_x, player_y);
}
