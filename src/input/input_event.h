/**
 * @file input_event.h
 */
#ifndef GINC_ROGUELIKE_INPUT_EVENT_H
#define GINC_ROGUELIKE_INPUT_EVENT_H

#include <SDL3/SDL_events.h>

struct inpt_state;

/**
 * Update the input state based on an SDL event.
 *
 * @param istate The input state to update.
 * @param event  The event to process.
 *
 * @return whether istate was updated.
 */
bool
inpt_handle_event(struct inpt_state* istate, SDL_Event const* event);

#endif // GINC_ROGUELIKE_INPUT_EVENT_H
