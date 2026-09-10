/**
 * @file controls.h
 */
#ifndef GINC_ROGUELIKE_CONTROLS_H
#define GINC_ROGUELIKE_CONTROLS_H

#include <SDL3/SDL_rect.h>

#include "action.h"

// forward declarations
struct inpt_state;
struct rl_map_view;

/**
 * Translate the current input state into a game action.
 */
enum rl_action
rl_translate_input(struct inpt_state const* istate,
                   SDL_Point rogue_position,
                   struct rl_map_view const* map_view);

#endif // GINC_ROGUELIKE_CONTROLS_H
