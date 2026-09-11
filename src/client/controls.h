/**
 * @file controls.h
 */
#ifndef GINC_ROGUELIKE_CONTROLS_H
#define GINC_ROGUELIKE_CONTROLS_H

#include <SDL3/SDL_rect.h>

#include "action.h"

// forward declarations
struct inpt_state;

enum rl_action
rl_handle_keyboard_input(struct inpt_state const* istate);

enum rl_action
rl_handle_mouse_input(struct inpt_state const* istate,
                      SDL_Point rogue,
                      SDL_Point target);

#endif // GINC_ROGUELIKE_CONTROLS_H
