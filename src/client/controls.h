/**
 * @file controls.h
 */
#ifndef GINC_ROGUELIKE_CONTROLS_H
#define GINC_ROGUELIKE_CONTROLS_H

#include <SDL3/SDL_rect.h>

#include "game/action.h"

// forward declarations
struct inpt_state;

/**
 * Translate the current input state into a game action.
 */
enum rl_action
rl_translate_input(struct inpt_state const* istate, SDL_Point rogue_position);

#endif // GINC_ROGUELIKE_CONTROLS_H
