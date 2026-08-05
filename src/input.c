#include "input.h"

#define BUTTON_IS_DOWN_MASK (1u << 0)
#define BUTTON_WAS_DOWN_MASK (1u << 1)

void
inpt_set_button(inpt_button* button, bool is_down)
{
  bool const was_down = inpt_is_down(*button);
  *button =
    (is_down ? BUTTON_IS_DOWN_MASK : 0) | (was_down ? BUTTON_WAS_DOWN_MASK : 0);
}

bool
inpt_is_down(inpt_button button)
{
  return (button & BUTTON_IS_DOWN_MASK) != 0;
}

bool
inpt_was_pressed(inpt_button button)
{
  bool const is_down = (button & BUTTON_IS_DOWN_MASK) != 0;
  bool const was_down = (button & BUTTON_WAS_DOWN_MASK) != 0;
  return is_down && !was_down;
}

bool
inpt_was_released(inpt_button button)
{
  bool const is_down = (button & BUTTON_IS_DOWN_MASK) != 0;
  bool const was_down = (button & BUTTON_WAS_DOWN_MASK) != 0;
  return !is_down && was_down;
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
    inpt_set_button(&istate->keys[i], inpt_is_down(istate->keys[i]));
  }
}
