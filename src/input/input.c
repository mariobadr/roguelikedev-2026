#include "input.h"

#define BUTTON_IS_DOWN_MASK (1u << 0)
#define BUTTON_WAS_PRESSED_MASK (1u << 1)
#define BUTTON_WAS_RELEASED_MASK (1u << 2)

void
inpt_set_button(inpt_button* button, bool is_down)
{
  bool const was_down = inpt_is_down(*button);

  if (is_down == was_down) {
    return;
  }

  if (is_down) {
    *button |= BUTTON_IS_DOWN_MASK | BUTTON_WAS_PRESSED_MASK;
  } else {
    *button &= ~BUTTON_IS_DOWN_MASK;
    *button |= BUTTON_WAS_RELEASED_MASK;
  }
}

bool
inpt_is_down(inpt_button button)
{
  return (button & BUTTON_IS_DOWN_MASK) != 0;
}

bool
inpt_was_pressed(inpt_button button)
{
  return (button & BUTTON_WAS_PRESSED_MASK) != 0;
}

bool
inpt_was_released(inpt_button button)
{
  return (button & BUTTON_WAS_RELEASED_MASK) != 0;
}

void
inpt_init_state(struct inpt_state* istate)
{
  SDL_memset(istate, 0, sizeof(*istate));
}

void
inpt_reset_state(struct inpt_state* istate)
{
  for (int i = 0; i < SDL_SCANCODE_COUNT; ++i) {
    istate->keys[i] &= BUTTON_IS_DOWN_MASK;
  }

  for (int i = 0; i < INPUT_MOUSE_BUTTON_COUNT; ++i) {
    istate->mouse.buttons[i] &= BUTTON_IS_DOWN_MASK;
  }
}
