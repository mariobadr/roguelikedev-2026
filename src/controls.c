#include "controls.h"

#include "input.h"
#include "render.h"

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

  if (inpt_was_pressed(istate->keys[SDL_SCANCODE_R])) {
    action->type = RL_ACTION_GEN_MAP;
    return true;
  }

  return false;
}

/**
 * Update action based on mouse state.
 *
 * @param action the action to mutate
 * @param istate the current frame's input state
 * @param rogue  the position of the rogue
 *
 * @return whether action was mutated
 */
static bool
handle_mouse_input(struct rl_action* action,
                   struct inpt_state const* istate,
                   SDL_Point rogue)
{
  if (inpt_is_down(istate->mouse.buttons[SDL_BUTTON_LEFT])) {
    int const target_x =
      (int)SDL_floorf(istate->mouse.position.x / GLYPH_WIDTH);
    int const target_y =
      (int)SDL_floorf(istate->mouse.position.y / GLYPH_HEIGHT);

    int const delta_x = target_x - rogue.x;
    int const delta_y = target_y - rogue.y;

    if (delta_x == 0 && delta_y == 0) {
      // already at target
      return false;
    }

    // move along x- or y-axis, but not both
    if (SDL_abs(delta_x) > SDL_abs(delta_y)) {
      set_movement_action(action, sign(delta_x), 0);
    } else {
      set_movement_action(action, 0, sign(delta_y));
    }

    return true;
  }

  return false;
}



struct rl_action
rl_translate_input(struct inpt_state const* istate, SDL_Point rogue_position)
{
  struct rl_action action = { 0 };

  // prioritize keyboard input over mouse input (?)
  if (handle_keyboard_input(&action, istate)) {
    return action;
  }

  handle_mouse_input(&action, istate, rogue_position);

  return action;
}
