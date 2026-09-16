#include "input_event.h"

#include <SDL3/SDL_assert.h>

#include "input.h"

/**
 * Update a keyboard key based on an event.
 *
 * @param istate The input state to update.
 * @param event  The keyboard event to process.
 */
static void
handle_key_event(struct inpt_state* istate, SDL_KeyboardEvent const* event)
{
  inpt_set_button(&istate->keys[event->scancode], event->down);
}

/**
 * Update the mouse's position based on an event.
 *
 * @param istate The input state to update.
 * @param event  The mouse motion event to process.
 */
static void
handle_mouse_motion_event(struct inpt_state* istate,
                          SDL_MouseMotionEvent const* event)
{
  istate->mouse.position.x = event->x;
  istate->mouse.position.y = event->y;
}

/**
 * Update a mouse button based on an event.
 *
 * @param istate The input state to update.
 * @param event  The mouse button event to process.
 */
static void
handle_mouse_button_event(struct inpt_state* istate,
                          SDL_MouseButtonEvent const* event)
{
  inpt_set_button(&istate->mouse.buttons[event->button], event->down);

  istate->mouse.position.x = event->x;
  istate->mouse.position.y = event->y;
}

bool
inpt_handle_event(struct inpt_state* istate, SDL_Event const* event)
{
  SDL_assert(istate != NULL && event != NULL);

  switch (event->type) {
    case SDL_EVENT_MOUSE_MOTION:
      handle_mouse_motion_event(istate, &event->motion);
      return true;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
      handle_mouse_button_event(istate, &event->button);
      return true;

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
      handle_key_event(istate, &event->key);
      return true;

    default:
      break;
  }

  return false;
}
