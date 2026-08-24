#include "controls.h"

#include "input.h"
#include "render.h"

/**
 * @return -1 for negative, 1 for positive, and 0 for zero
 */
static int
sign(int number)
{
  return (number > 0) - (number < 0);
}

/**
 * @return the action based on keyboard state
 */
static enum rl_action
handle_keyboard_input(struct inpt_state const* istate)
{
  if (inpt_is_down(istate->keys[SDL_SCANCODE_W])) {
    return RL_ACTION_MOVE_UP;
  }

  if (inpt_is_down(istate->keys[SDL_SCANCODE_S])) {
    return RL_ACTION_MOVE_DOWN;
  }

  if (inpt_is_down(istate->keys[SDL_SCANCODE_A])) {
    return RL_ACTION_MOVE_LEFT;
  }

  if (inpt_is_down(istate->keys[SDL_SCANCODE_D])) {
    return RL_ACTION_MOVE_RIGHT;
  }

  if (inpt_was_pressed(istate->keys[SDL_SCANCODE_R])) {
    return RL_ACTION_DEBUG_GENMAP;
  }

  return RL_ACTION_NONE;
}

/**
 * @return the action based on mouse state
 */
static enum rl_action
handle_mouse_input(struct inpt_state const* istate, SDL_Point rogue)
{
  if (!inpt_is_down(istate->mouse.buttons[SDL_BUTTON_LEFT])) {
    return RL_ACTION_NONE;
  }

  int const target_x = (int)SDL_floorf(istate->mouse.position.x / GLYPH_WIDTH);
  int const target_y = (int)SDL_floorf(istate->mouse.position.y / GLYPH_HEIGHT);

  int const delta_x = target_x - rogue.x;
  int const delta_y = target_y - rogue.y;

  if (delta_x == 0 && delta_y == 0) {
    // already at target
    return RL_ACTION_NONE;
  }

  // move along x- or y-axis, but not both
  if (SDL_abs(delta_x) > SDL_abs(delta_y)) {
    if (sign(delta_x) > 0) {
      return RL_ACTION_MOVE_RIGHT;
    }
    return RL_ACTION_MOVE_LEFT;
  }

  if (sign(delta_y) > 0) {
    return RL_ACTION_MOVE_DOWN;
  }
  return RL_ACTION_MOVE_UP;
}

enum rl_action
rl_translate_input(struct inpt_state const* istate, SDL_Point rogue_position)
{
  // prioritize keyboard input over mouse input (?)
  enum rl_action const action = handle_keyboard_input(istate);
  if (action != RL_ACTION_NONE) {
    return action;
  }

  return handle_mouse_input(istate, rogue_position);
}
