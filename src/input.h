/**
 * @file input.h
 */
#ifndef GINC_ROGUELIKE_INPUT_H
#define GINC_ROGUELIKE_INPUT_H

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>

/**
 * Encoded state of a button, capturing both the current and previous-frame
 * state.
 */
typedef Uint8 inpt_button;

/**
 * The state of all inputs captured during a single frame.
 */
struct inpt_state
{
  /** Keyboard key state indexed by SDL_Scancode. */
  inpt_button keys[SDL_SCANCODE_COUNT];
};

/**
 * Update button state based on the current physical state.
 *
 * @param button  The button state to update.
 * @param is_down true if the button is held down, false otherwise.
 */
void
inpt_set_button(inpt_button* button, bool is_down);

/**
 * Check whether a button is currently held down.
 *
 * @param button The button state to check.
 *
 * @return true if the button is held, false otherwise.
 */
bool
inpt_is_down(inpt_button button);

/**
 * Check whether a button was pressed this frame.
 *
 * @param button The button state to check.
 *
 * @return true if the button transitioned from up to down, false otherwise.
 */
bool
inpt_was_pressed(inpt_button button);

/**
 * Check whether a button was released this frame.
 *
 * @param button The button state to check.
 *
 * @return true if the button transitioned from down to up, false otherwise.
 */
bool
inpt_was_released(inpt_button button);

/**
 * Initialize the input state.
 *
 * @param istate The input state to initialize.
 */
void
inpt_init_state(struct inpt_state* istate);

/**
 * Reset transient input state, preparing it for the next frame.
 *
 * @param istate The input state to reset.
 */
void
inpt_reset_state(struct inpt_state* istate);

#endif // GINC_ROGUELIKE_INPUT_H
