#include "controls.h"

#include "input/input.h"

#include "client/view/map.h"

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
enum rl_action
rl_handle_keyboard_input(struct inpt_state const* istate)
{
  // "is down" actions
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

  // "was pressed" actions
  if (inpt_was_pressed(istate->keys[SDL_SCANCODE_E])) {
    return RL_ACTION_SELECT;
  }

  if (inpt_was_pressed(istate->keys[SDL_SCANCODE_TAB])) {
    return RL_ACTION_FOCUS_NEXT;
  }

  if (inpt_was_pressed(istate->keys[SDL_SCANCODE_U])) {
    return RL_ACTION_DEBUG_USE_ITEM;
  }

  return RL_ACTION_NONE;
}

/**
 * @return the action based on mouse state
 */
enum rl_action
rl_handle_mouse_input(struct inpt_state const* istate,
                   SDL_Point rogue,
                   struct rl_map_view const* map_view)
{
  if (!inpt_is_down(istate->mouse.buttons[SDL_BUTTON_LEFT])) {
    return RL_ACTION_NONE;
  }

  SDL_Point target = { 0 };
  if (!rl_map_view_cell_at(map_view, istate->mouse.position, &target)) {
    return RL_ACTION_NONE;
  }

  int const delta_x = target.x - rogue.x;
  int const delta_y = target.y - rogue.y;

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
